#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <unistd.h>

#include "actuators/dotmatrix.h"
#include "actuators/lcd.h"
#include "sensors/dht11.h"
#include "sensors/soilmoisture.h"
#include "sensors/photoresistor.h"

// 순간 측정했을 때 값이 튀는 걸 방지하여, 주기적으로 측정해야 되는데 DHT11같은 경우는 처음부터 값 측정 오류가 나는 경우가 빈번하므로
// 초기 3회 측정 후 평균값을 내는 걸 토대로, 3회 측정에 대한 평균값이 식물 성장 환경에서의 평균치에 미치지 못할 경우 다시 재측정?

// 식물과의 상호작용은 How?

// 식물의 상태에 따른 스피커 안내음성 출력 - 기능별 분류(1)
// 적외선 센서로 사람을 감지하면 

char status[100] = ""; //현재 식물 상태 저장 배열 전역변수

float DHT11_temp;
float soilmoisture;
int is_bright;

void detect_noise(void* value, int sensor){
//센서값이 튀는 것을 감지해 적절한 값으로 설정해주는 함수.
//value : 센서의 측정 값,  sensor : 센서 종류 (0:온도, 1:토양습도, 2:조도)
    float current_temp;
    float current_soilmost;
    int current_bright;
    switch(sensor){
        case 0: //온도센서 (1시간 단위 측정을 기준으로 만듬)
            current_temp = *(float*)value;
            if(abs(DHT11_temp - current_temp) > 5.0){
                if(abs(current_temp - get_temperature_five_times()) < 1.0){
                    DHT11_temp = current_temp;
                } //측정한 온도 값이 이전 값과 5도 이상 차이 날 경우, 다시 온도를 측정한다.
                  //만약 다시 측정한 값도 기존 값과 비슷한 차이를 보일 경우, 그 값을 적용한다.
                  //측정한 온도 값이 튄 값으로 판단될 경우, 값을 업데이트 하지 않는다.
            }
            else{
                DHT11_temp = current_temp; //기존 온도값과 측정 온도값이 5도 이하의 차이라면
                                           //그 값으로 온도값을 업데이트 한다.
            }
        case 1: //토양습도센서 (12시간 단위 측정을 기준으로 만듬)
            current_soilmost = *(float*)value;
            if(abs(soilmoisture - current_soilmost) > 25.0){ //12시간 후 측정 습도가 25%차라면
                if(abs(current_temp - get_temperature_five_times()) < 5.0){
                    DHT11_temp = current_temp;
                } //측정한 온도 값이 이전 값과 5도 이상 차이 날 경우, 다시 온도를 측정한다.
                  //만약 다시 측정한 값도 기존 값과 비슷한 차이를 보일 경우, 그 값을 적용한다.
                  //측정한 온도 값이 튄 값으로 판단될 경우, 값을 업데이트 하지 않는다.
            }
            else{
                DHT11_temp = current_temp; //기존 온도값과 측정 온도값이 5도 이하의 차이라면
                                           //그 값으로 온도값을 업데이트 한다.
            }


    }
}

void *t_temperature(){
    while(1){ //온도 센서의 적당한 작동 주기로 설정 필요
        float temp;
        temp = get_temperature_five_times();    
        detect_noise(&temp, 0); //튀는 값 잡고, 전역변수에 온도값 반영.

        DHT11_temp = get_temperature();

        if (DHT11_temp <= 18.0)
            {
                strcat(status, "Cold!");
                printf("[!] 온도: 너무 춥습니다.\n");
            }
            else if (DHT11_temp >= 35.0)
            {
                strcat(status, "Hot!");
                printf("[!] 온도: 너무 덥습니다.\n");
            }
            printf("현재 온도: %.1f°C\n", DHT11_temp);
    }
}

void *t_soilmoisture(){
    while(1){//토양 습도 센서의 적당한 작동 주기 설정
        soilmoisture = get_soilmoisture();
            if (soilmoisture <= 20.0)
            {
                strcat(status, "Dry!");
                printf("[!] 토양습도: 너무 건조합니다.\n");
            }
        printf("현재 토양습도 : %.2f%%\n", soilmoisture);
    }
}

void *t_brightness(){
    while(1){//조도 센서의 적당한 작동 주기 설정
        is_bright = get_brightness();
        if (is_bright)
        {
            printf("현재 밝기: 밝음\n");
        }
        else
        {
            strcat(status, "Dark!");
            printf("[!] 밝기: 너무 어둡습니다.\n");
            printf("현재 밝기: 어두움\n");
        }
    }
}

void *t_LCD_Dot(){
    while (1){//센서의 작동 주기에 따라 액추에이터 작동 주기도 설정 필요. (센서 작동과 동기화 한다던지...)
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

        //delay(1000);
    }
}


int main(void)
{
    if (wiringPiSetup() == -1)
        return -1;

    dotmatrix_init();

    pthread_t threadId[4];
    int thread_status;
    int thread_join_status;
    thread_status = (&threadId[0], NULL, t_temperature, NULL); //온도센서 쓰레드 생성
    if(thread_status < 0){
        printf("온도센서 쓰레드 생성 오류\n");
        exit(0);
    }
    thread_status = (&threadId[1], NULL, t_soilmoisture, NULL); //토양습도센서 쓰레드 생성
    if(thread_status < 0){
        printf("토양습도센서 쓰레드 생성 오류\n");
        exit(0);
    }
    thread_status = (&threadId[2], NULL, t_brightness, NULL); //조도센서 쓰레드 생성
    if(thread_status < 0){
        printf("조도센서 쓰레드 생성 오류\n");
        exit(0);
    }
    thread_status = (&threadId[3], NULL, t_LCD_Dot, NULL); //LCD, 도트매트릭스 쓰레드 생성
    if(thread_status < 0){
        printf("LCD, 도트 매트릭스 쓰레드 생성 오류\n");
        exit(0);
    }


    pthread_join(threadId[0], thread_join_status);
    pthread_join(threadId[1], thread_join_status);
    pthread_join(threadId[2], thread_join_status);
    pthread_join(threadId[3], thread_join_status);
    return 0;
}