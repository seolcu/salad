#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>


volatile int eventCounter = 0; // Event counter for motion detection
#define MOTION_IN 0 //WiringPi 기준 0번 핀 (GPIO 17번 핀)

// Interrupt service routine
void myInterrupt(void)
{
    eventCounter++;
}

int main(void)
{
    //WiringPi 초기화
    if (wiringPiSetup() < 0)
    {
        printf("MotionSensor : wiringPi 셋업에 실패했습니다.\n");
        return 1;
    }

    // Set pin mode and attach interrupt
    pinMode(MOTION_IN, INPUT);
    if (wiringPiISR(MOTION_IN, INT_EDGE_RISING, &myInterrupt) < 0)
    {
        printf("Unable to setup ISR\n");
        return 1;
    }

    printf("Monitoring motion sensor...\n");

    while (1)
    {
        if (digitalRead(MOTION_IN) > 0)
        {
            printf("Infrared Sensor Detected! Total Events: %d\n", eventCounter);
            eventCounter = 0; // Reset counter after processing
            // 사람 감지 시 수행할 로직 작성 부분
        }
        else
        {
            printf("No motion detected.\n");
        }
        delay(500); // Delay for 0.5 seconds
    }

    return 0;
}