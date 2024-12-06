#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "actuators/lcd.h"
#include "sensors/dht11.h"
#include "sensors/soilmoisture.h"
#include "sensors/photoresistor.h"

int main(void)
{
    if (wiringPiSetup() == -1)
        return -1;

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
        if (soilmoisture <= 15.0)
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
            set_lcd_text(status);
        }
        else
        {
            set_lcd_text("Well Done!");
        }

        delay(2000);
    }

    return 0;
}