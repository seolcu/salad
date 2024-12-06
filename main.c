#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include "sensors/DHT11.h"

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
        delay(3000);
    }

    return 0;
}