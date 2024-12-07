#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "actuators/dotmatrix.h"
#include "actuators/lcd.h"
#include "sensors/dht11.h"
#include "sensors/soilmoisture.h"
#include "sensors/photoresistor.h"

// 순간 측정했을 때 값이 튀는 걸 방지하여, 주기적으로 측정해야 되는데 DHT11같은 경우는 처음부터 값 측정 오류가 나는 경우가 빈번하므로
// 초기 3회 측정 후 평균값을 내는 걸 토대로, 3회 측정에 대한 평균값이 식물 성장 환경에서의 평균치에 미치지 못할 경우 다시 재측정?

// 식물과의 상호작용은 How?

// 토양습도 측정 주기의 경우, 점토질 토양에 물을 줄 때 완전히 스며들어 없어지는 시간까지 평균적으로 2~3일이므로 넉넉하게 24시간에 한 번씩
// 측정해도 될 것이라고 판단.

// 채소 및 과일: 41% - 80% (예: 딸기, 토마토 등)
// 다육식물 및 선인장: 20% - 40% (건조한 환경에 적합)
// 관엽식물: 21% - 60% (종류에 따라 다름)

int main(void)
{
    if (wiringPiSetup() == -1)
        return -1;

    dotmatrix_init();

    while (1)
    {
        char status[100] = "";

        float DHT11_temp = get_temperature();

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

        float soilmoisture = get_soilmoisture();
        if (soilmoisture <= 20.0)
        {
            strcat(status, "Dry!");
            printf("[!] 토양습도: 너무 건조합니다.\n");
        }
        printf("현재 토양습도 : %.2f%%\n", soilmoisture);

        int is_bright = get_brightness();

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

        delay(1000);
    }

    return 0;
}