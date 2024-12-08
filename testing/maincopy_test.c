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

// 순간 측정했을 때 값이 튀는 걸 방지하여, 주기적으로 측정해야 되는데 DHT11같은 경우는 처음부터 값 측정 오류가 나는 경우가 빈번하므로
// 초기 3회 측정 후 평균값을 내는 걸 토대로, 3회 측정에 대한 평균값이 식물 성장 환경에서의 평균치에 미치지 못할 경우 다시 재측정? (해결)

// 식물과의 상호작용은 How?

// 식물의 상태에 따른 스피커 안내음성 출력 - 기능별 분류(1)
// 적외선 센서로 사람을 감지하면 

char status[100] = "";
char status_temp[100] = ""; //현재 식물 상태 저장 배열 전역변수
char status_soilmoist[100] = "";
char status_bright[100] = "";



float DHT11_temp = 0.0;
float soilmoisture;
int is_bright;

int update = 0;

float abs_fl(float num1, float num2)
{
    if((num1 - num2) < 0)
    {
        return -(num1 - num2);
    }

    else if((num1 - num2) > 0)
    {
        return (num1 - num2);
    }

    else
    {
        return 0;
    }
}

void detect_noise(void* value, int sensor){
//센서값이 튀는 것을 감지해 적절한 값으로 설정해주는 함수.
//value : 센서의 측정 값,  sensor : 센서 종류 (0:온도, 1:토양습도)
    float current_temp;
    float second_temp;
    //float current_soilmost;
    
    switch(sensor){
        case 0: //온습도센서 (1시간 단위 측정을 기준으로 만듬)
            current_temp = *(float*)value;
            if(abs_fl(DHT11_temp, current_temp) > 5.0){
                second_temp = get_temperature();
                if(abs_fl(current_temp, second_temp) < 1.0){
                    DHT11_temp = current_temp;
                } 
                else{
                    DHT11_temp = second_temp;
                }//측정한 온도 값이 이전 값과 5도 이상 차이 날 경우, 다시 온도를 측정한다.
                  //만약 다시 측정한 값도 기존 값과 비슷한 차이를 보일 경우, 그 값을 적용한다.
                  //측정한 온도 값(첫 번째 측정한 값(current_temp))이 튄 값으로 판단될 경우, 한 번 더 측정한 값으로 업데이트해준다.
            }
            else{
                DHT11_temp = current_temp; //기존 온도값과 측정 온도값이 5도 이하의 차이라면
                                           //그 값으로 온도값을 업데이트 한다.
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
        // 튀는 값 기준 정하는게 빡세서 일단 보류함
    }
}

void *t_temperature(){
    while(1){ //온도 센서의 적당한 작동 주기로 설정 필요
        float temp;
        temp = get_temperature();    
        detect_noise(&temp, 0); //튀는 값 잡고, 전역변수에 온도값 반영.
        update = 1;
        //DHT11_temp = get_temperature();

        if (DHT11_temp <= 18.0)
        {
            strcpy(status_temp, "Cold!");
            printf("[!] 온도 : 너무 춥습니다.\n");
        }
        else if (DHT11_temp >= 35.0)
        {
            strcpy(status_temp, "Hot!");
            printf("[!] 온도 : 너무 덥습니다.\n");
        }
        printf("현재 온도 : %.1f°C\n", DHT11_temp);
        delay(30000); //1시간마다 온도 측정, 테스트를 위해 시간을 짧게 만들어 놓음.
    }
}

void *t_soilmoisture(){
    while(1){//토양 습도 센서의 적당한 작동 주기 설정
        soilmoisture = get_soilmoisture();
        if (soilmoisture <= 20.0)
        {
            strcpy(status_soilmoist, "Dry!");
            printf("[!] 토양습도 : 너무 건조합니다.\n\n");
        }
        else{
            strcpy(status_soilmoist, "");
        }
        printf("현재 토양습도 : %.2f%%\n", soilmoisture);
        update = 1;
        delay(60000); //12시간마다 토양습도 측정, 테스트를 위해 시간을 짧게 만들어 놓음.
    }
}

void *t_brightness(){
    while(1){//조도 센서의 적당한 작동 주기 설정
        is_bright = get_brightness();
        if (is_bright)
        {
            strcpy(status_bright, "");
            printf("현재 밝기: 밝음\n");
        }
        else
        {
            strcpy(status_bright, "Dark!");
            printf("[!] 밝기: 너무 어둡습니다.\n");
            printf("현재 밝기: 어두움\n");
        }
        update = 1;
        delay(20000); //1시간마다 밝기 측정, 테스트를 위해 시간을 짧게 만들어 놓음.
    }
}

void *t_LCD_Dot(){
    while(1){
        if(update == 1){ //3종류의 센서 중 하나라도 값이 업데이트 되었다면 이 변수가 1이 되고, LCD, 도트매트릭스에 정보를 업데이트 한다.
            strcat(status, status_temp);
            strcat(status, status_soilmoist);
            strcat(status, status_bright);

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
            update = 0;
        }
    }
}

void update_sensors(){
    float temp;
    temp = get_temperature();    //온도 최신화
    detect_noise(&temp, 0);
    printf("온도     최신화, 온도 : %.1f\n", DHT11_temp);
    if (DHT11_temp <= 18.0)
    {
        strcpy(status_temp, "Cold!");
    }
    else if (DHT11_temp >= 35.0)
    {
        strcpy(status_temp, "Hot!");
    }

    soilmoisture = get_soilmoisture();      //토양습도 최신화
    printf("토양습도 최신화, 습도 : %.2f\n", soilmoisture);  
    if (soilmoisture <= 20.0)
    {
        strcpy(status_soilmoist, "Dry!");
    }
    else{
        strcpy(status_soilmoist, "");
    }

    is_bright = get_brightness();           //조도 최신화
    printf("밝기     최신화, 조도 : %d\n", is_bright);
    if (is_bright)
    {
        strcpy(status_bright, "");
    }
    else
    {
        strcpy(status_bright, "Dark!");
    }
}

void *t_motion(){
    int motion_detected = 0;
    while(1)
    {
        motion_detected = enable_motionsensor();
        if(motion_detected)
        { //만약 적외선 모션센서에 움직임이 감지된다면,
            
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
            //적외선 모션 감지 시 추가로 수행할 내용은 아래에 작성.
            delay(20000);
            motion_detected = 0;
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