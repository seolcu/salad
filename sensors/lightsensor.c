#include <wiringPi.h>
#include <stdio.h>

#define LDR_PIN 2 // GPIO 핀 번호 (wiringPi 기준, GPIO2, PHYSICAL : 13)

int main(void)
{
    // WiringPi 초기화
    if (wiringPiSetup() == -1)
    {
        printf("WiringPi 초기화 실패!\n");
        return 1;
    }

    // LDR_PIN을 입력 모드로 설정
    pinMode(LDR_PIN, INPUT);

    while (1)
    {
        // GPIO 핀에서 값을 읽음
        int lightStatus = digitalRead(LDR_PIN);

        if (lightStatus == HIGH)
        {
            printf("밝음\n"); // HIGH: 어두운 상태
        }
        else
        {
            printf("어두움\n"); // LOW: 밝은 상태
        }

        delay(3000); // 1초 대기
    }

    return 0;
}