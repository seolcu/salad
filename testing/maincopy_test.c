#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <unistd.h>

#include "../actuators/dotmatrix.h"
#include "../actuators/lcd.h"
#include "../sensors/dht11.h"
#include "../sensors/soilmoisture.h"
#include "../sensors/photoresistor.h"
#include "../sensors/motionsensor.h"

char status[100] = "";
char status_temp[100] = ""; //현재 식물 상태 저장 배열 전역변수
char status_soilmoist[100] = "";
char status_bright[100] = "";

float DHT11_temp = 0.0;
float soilmoisture;
int is_bright;

int update = 0;

// (만약 음성 출력까지 구현했을 때) 주기적 측정과 순간적 측정 타이밍이 겹칠 경우, 두 케이스에 대한 모든 음성 출력이 겹칠 텐데 이를 어떻게 처리할 것인가.

// 토양습도 측정 주기의 경우, 점토질 토양에 물을 줄 때 완전히 스며들어 없어지는 시간까지 평균적으로 2~3일이므로 넉넉하게 24시간에 한 번씩
// 측정해도 될 것이라고 판단.

// 채소 및 과일: 41% - 80% (예: 딸기, 토마토 등)
// 다육식물 및 선인장: 20% - 40% (건조한 환경에 적합)
// 관엽식물: 21% - 60% (종류에 따라 다름)

// 시간별 (토양)습도 변화
// 다음은 점토질 토양에서 물을 흡수한 후 시간별로 예상되는 습도 변화입니다
// 시간 경과	토양 습도 (%)	설명
// 0~1시간	20~30%	초기 건조 상태. 물이 표면에서 빠르게 흡수되기 시작.
// 1~3시간	30~50%	물이 점차 깊은 층으로 스며들며, 습도가 증가.
// 3~6시간	50~70%	흙의 중간 층까지 물이 도달하며, 점토가 포화 상태에 가까워짐.
// 6~24시간	70~90%	거의 포화 상태에 도달하며, 잔류 수분이 천천히 이동.
// 24시간 이후	90~100%	포화 상태에 도달. 더 이상의 물 흡수는 제한적이며 배수가 시작될 수 있음.


float abs_fl(float num1, float num2) // | num1 - num2 |에 대한 절댓값을 구하는 함수
{
    if((num1 - num2) < 0)
    {
        return -(num1 - num2);
    }

    else if((num1 - num2) > 0)
    {
        return (num1 - num2);
    }

    else // num1 - num2 = 0인 경우
    {
        return 0;
    }
}

void detect_noise(void* value) // 센서값이 튀는 것을 감지하여 적절한 값으로 설정해주는 함수, value : 센서의 측정 값
{
    float current_temp;
    float second_temp;

    current_temp = *(float*)value;

    if(abs_fl(DHT11_temp, current_temp) > 5.0)
    {
        second_temp = get_temperature();

        if(abs_fl(current_temp, second_temp) < 1.0)
        {
            DHT11_temp = current_temp;
        } 

        else
        {
            DHT11_temp = second_temp;
        }
        //측정한 온도 값이 이전 값과 5도 이상 차이 날 경우, 다시 온도를 측정한다.
        //만약 다시 측정한 값도 기존 값과 비슷한 차이를 보일 경우, 그 값을 적용한다. = 튄 것이 아닌 환경 변화로 인한 급격한 오도 변화로 판단
        //측정한 온도 값(첫 번째 측정한 값(current_temp))이 튄 값으로 판단될 경우, 한 번 더 측정한 값으로 업데이트해준다.
     }
    
    else
    {
        DHT11_temp = current_temp; //기존 온도값과 측정 온도값이 5도 이하의 차이라면 그 값으로 온도값을 업데이트 한다.
    }
        // case 1: //토양습도센서 (12시간 단위 측정을 기준으로 만듬)
        //     current_soilmost = *(float*)value;
        //     if(abs(soilmoisture - current_soilmost) > 25.0){ //12시간 후 측정 습도가 25%차 라면
        //         if(abs(current_temp - get_temperature_five_times()) < 5.0){
        //             DHT11_temp = current_temp;
        //         } //측정한 온도 값이 이전 값과 5도 이상 차이 날 경우, 다시 온도를 측정한다.
        //           //만약 다시 측정한 값도 기존 값과 비슷한 차이를 보일 경우, 그 값을 적용한다.
        //           //측정한 온도 값이 튄 값으로 판단될 경우, 값을 업데이트 하지 않는다.
        //     }
        //     else{
        //         DHT11_temp = current_temp; //기존 온도값과 측정 온도값이 5도 이하의 차이라면
        //                                    //그 값으로 온도값을 업데이트 한다.
        //     }
        // 구현하려고 하였으나 토양습도센서의 경우, 값이 튀는 경우가 없어 그냥 뺐다.
}

void *t_temperature()
{
    while(1)
    { 
        float temp;
        temp = get_temperature();    
        detect_noise(&temp); //튀는 값 잡고, 전역변수에 온도값 반영.
        int event = 0; 
        update = 1;

        if (DHT11_temp <= 16.0)
        {
            strcpy(status_temp, "Cold!");
            printf("------- 온도 측정결과 -------\n\n     [!] 온도 : 너무 춥습니다.\n");
            event = 1;
        }

        else if (DHT11_temp >= 35.0)
        {
            strcpy(status_temp, "Hot!");
            printf("-------- 온도 측정결과 ---------\n\n     [!] 온도 : 너무 덥습니다.\n");
            event = 1;
        }

        if(event == 1)
        {
            printf("     현재 온도 : %.1f°C\n\n", DHT11_temp);
        }
        
        else
        {
            printf("-------- 온도 측정결과 ---------\n\n     현재 온도 : %.1f°C\n\n", DHT11_temp);
        }
        printf("--------------------------------\n\n");
        delay(30000); //1시간마다 온도 측정, 테스트를 위해 시간을 짧게 만들어 놓음.
    }
}

void *t_soilmoisture()
{
    while(1)
    {
        soilmoisture = get_soilmoisture();

        if (soilmoisture <= 20.0)
        {
            strcpy(status_soilmoist, "Dry!");
            printf("------ 토양습도 측정결과 -------\n\n [!] 토양습도 : 너무 건조합니다.\n");
        }

        else
        {
            strcpy(status_soilmoist, "");
            printf("------  토양습도 측정결과 -------\n\n");
        }

        printf("   현재 토양습도 : %.2f%%\n\n", soilmoisture);
        printf("--------------------------------\n\n");
        update = 1;
        delay(60000); //12시간마다 토양습도 측정, 테스트를 위해 시간을 짧게 만들어 놓음.
    }
}

void *t_brightness()
{
    while(1)
    {
        is_bright = get_brightness();

        if (is_bright)
        {
            strcpy(status_bright, "");
            printf("----- 조도(밝기) 측정결과 ------\n\n      현재 밝기: 밝음\n\n");
        }

        else
        {
            strcpy(status_bright, "Dark!");
            printf("----- 조도(밝기) 측정결과 ------\n\n");
            printf("[!] 밝기: 너무 어둡습니다.\n");
            printf("      현재 밝기: 어두움\n\n");
        }

        printf("--------------------------------\n\n");
        update = 1;
        delay(20000); //1시간마다 밝기 측정, 테스트를 위해 시간을 짧게 만들어 놓음.
    }
}

void *t_LCD_Dot(){
    while(1)
    {
        if(update == 1)
        { //3종류의 센서 중 하나라도 값이 업데이트 되었다면 이 변수가 1이 되고, LCD, 도트매트릭스에 정보를 업데이트 한다.
            strcat(status, status_temp);
            strcat(status, status_soilmoist);
            strcat(status, status_bright);

            if(strcmp(status, "") != 0) // 만약 식물의 온도, 토양습도, 조도 중 하나라도 문제가 있을 경우 status 배열 안에
            //어떠한 값이 들어있을 것이므로 0이 아닐 것이다. 따라서, 이런 경우 bad condition으로 
            {
                draw_sad();
                set_lcd_text(status);
            }

            else
            {
                draw_smile();
                set_lcd_text("Well Done!");
            }

            strcpy(status, "");
            update = 0;
        }
    }
}

void update_sensors()
{
    float temp;
    temp = get_temperature();    //온도 최신화
    detect_noise(&temp);

    printf("------- 측정값 최신화 -------\n\n");
    printf("        온도 : %.1f\n", DHT11_temp);

    if (DHT11_temp <= 18.0)
    {
        strcpy(status_temp, "Cold!");
    }
    else if (DHT11_temp >= 30.0)
    {
        strcpy(status_temp, "Hot!");
    }

    soilmoisture = get_soilmoisture();      //토양습도 최신화
    printf("    토양습도 : %.2f\n", soilmoisture);  

    if (soilmoisture <= 20.0)
    {
        strcpy(status_soilmoist, "Dry!");
    }
    else{
        strcpy(status_soilmoist, "");
    }

    is_bright = get_brightness();           //조도 최신화
    printf("        조도 : %d\n\n", is_bright);
    printf("--------------------------------\n\n");

    if (is_bright)
    {
        strcpy(status_bright, "");
    }
    else
    {
        strcpy(status_bright, "Dark!");
    }
}

void *t_motion()
{
    int motion_detected = 0;

    while(1)
    {
        motion_detected = enable_motionsensor();

        if(motion_detected) //만약 적외선 모션센서에 움직임이 감지된다면,
        { 
            update_sensors();
            strcat(status, status_temp);
            strcat(status, status_soilmoist);
            strcat(status, status_bright);
            //최신화된 값으로 LCD, 도트 출력 최신화

            if (strcmp(status, "") != 0)
            {
                draw_sad();
                set_lcd_text(status);
            }

            else
            {
                draw_smile();
                set_lcd_text("Well Done!");
            }

            strcpy(status, "");

            //**************** 적외선 모션 감지 후 조건에 따라 스피커에서 출력될 음성 관련 코드 부분 (공백) ******************

            motion_detected = 0;
            delay(20000); // delay가 너무 짧을 경우, 사용자 감지 직후 쓸데없이 상태를 다시 바로 알릴 수 있으므로 적절한 시간을 설정하여
            //적당한 시간이 지나고 다시 식물의 상태를 updata하여 알릴 수 있게끔 해준다.
        }
    }
}


int main(void)
{
    if (wiringPiSetup() == -1)
        return -1;

    dotmatrix_init();

    pthread_t threadId[5];
    int thread_status;
    int thread_join_status;
    thread_status = pthread_create(&threadId[0], NULL, t_temperature, NULL); //온도센서 쓰레드 생성
    if(thread_status < 0){
        printf("온도센서 쓰레드 생성 오류\n");
        exit(0);
    }
    thread_status = pthread_create(&threadId[1], NULL, t_soilmoisture, NULL); //토양습도센서 쓰레드 생성
    if(thread_status < 0){
        printf("토양습도센서 쓰레드 생성 오류\n");
        exit(0);
    }
    thread_status = pthread_create(&threadId[2], NULL, t_brightness, NULL); //조도센서 쓰레드 생성
    if(thread_status < 0){
        printf("조도센서 쓰레드 생성 오류\n");
        exit(0);
    }
    thread_status = pthread_create(&threadId[3], NULL, t_LCD_Dot, NULL); //LCD, 도트매트릭스 쓰레드 생성
    if(thread_status < 0){
        printf("LCD, 도트 매트릭스 쓰레드 생성 오류\n");
        exit(0);
    }
    thread_status = pthread_create(&threadId[4], NULL, t_motion, NULL); //적외선 모션센서 쓰레드 생성
    if(thread_status < 0){
        printf("적외선 모션센서 쓰레드 생성 오류\n");
        exit(0);
    }

    pthread_join(threadId[0], (void **)&thread_join_status);
    pthread_join(threadId[1], (void **)&thread_join_status);
    pthread_join(threadId[2], (void **)&thread_join_status);
    pthread_join(threadId[3], (void **)&thread_join_status);
    pthread_join(threadId[4], (void **)&thread_join_status);
    return 0;
}