#include <wiringPi.h>
#include <stdio.h>

#define LDR_PIN 2 // GPIO 핀 번호 (wiringPi 기준, GPIO2, PHYSICAL : 13)

int main(void)
{
    // WiringPi 초기화
    if (wiringPiSetup() == -1)
    {
        return -1;
    }

    // LDR_PIN을 입력 모드로 설정
    pinMode(LDR_PIN, INPUT);

    while (1)
    {
        // GPIO 핀에서 값을 읽음
        int light_val = digitalRead(LDR_PIN);

        if (light_val == HIGH)
        {
            printf("Brightness\n"); // HIGH: 밝은 상태
        }
        else
        {
            printf("Darkness\n"); // LOW: 어두운 상태
        }

        delay(3000); // 3초 대기
    }

    return 0;
}