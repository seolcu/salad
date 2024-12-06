#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdio.h>
#include <stdlib.h>
#include "sensors/dht11.h"
#include "sensors/soilmoisture.h"
#include "sensors/photoresistor.h"

int main(void)
{
    while (1)
    {
        if (wiringPiSetup() == -1)
            return -1;

        float DHT11_temp = get_temperature();

        if (DHT11_temp <= 18.0 || DHT11_temp >= 35.0)
        {
            printf("Plant's condition will be bad. The current temperature is %.1f°C\n", DHT11_temp);
        }

        else
        {
            printf("Good condition. The current temperature is %.1f°C\n", DHT11_temp);
        }

        float soilmoisture = get_soilmoisture();

        printf("토양습도 : %.2f%%\n", soilmoisture);

        int is_bright = get_brightness();

        if (is_bright)
        {
            printf("밝아요\n");
        }
        else
        {
            printf("어두워요\n");
        }

        delay(3000);
    }

    return 0;
}