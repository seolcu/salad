#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include "sensors/DHT11.h"

int main(void)
{
    while(1)
    {
        if (wiringPiSetup() == -1)
        return -1;

        DHT11_Data val = readData();

        if (val.temperature <= 18 || val.temperature >= 25)
        {
            printf("Plant's condition will be bad. The current temperature is %.1f°C\n", val.temperature);
        }

        else
        {
            printf("Good condition. The current temperature is %.1f°C\n", val.temperature);
        }
        delay(3000);
    }

    return 0;
}