#include <stdio.h>
#include <wiringPi.h>

void delay_second(int second)
{
    int i;
    for (i = 0; i < second; i++)
    {
        delay(1000);
    }
}

void delay_minute(int minute)
{
    int i;
    for (i = 0; i < minute; i++)
    {
        delay_second(60);
    }
}

void delay_hour(int hour)
{
    int i;
    for (i = 0; i < hour; i++)
    {
        delay_minute(60);
    }
}