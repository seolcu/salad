#include <wiringPi.h>
#include <stdio.h>

#define PIN 2 // 라즈베리파이 (13) `GPIO. 2`

// 조도센서에서 빛 유무를 받아오는 함수. 밝은 상태면 1, 어두운 상태면 0을 반환한다. 조도 센서가 밝음과 어두움을 구별하는 기준은 식물의 광합성 조건과 비슷하다.
int get_brightness()
{
    // LDR_PIN을 입력 모드로 설정
    pinMode(PIN, INPUT);
    int light_val = digitalRead(PIN);
    return light_val;
}

// int main(void)
// {
//     // WiringPi 초기화
//     if (wiringPiSetup() == -1)
//     {
//         return -1;
//     }

//     pinMode(PIN, INPUT);

//     while (1)
//     {
//         // GPIO 핀에서 값을 읽음
//         int light_val = digitalRead(PIN);

//         if (light_val == HIGH)
//         {
//             printf("Brightness\n"); // HIGH: 밝은 상태
//         }
//         else
//         {
//             printf("Darkness\n"); // LOW: 어두운 상태
//         }

//         delay(3000); // 3초 대기
//     }

//     return 0;
// }