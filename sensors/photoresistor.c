#include <wiringPi.h>
#include <stdio.h>
#include "../utility/write_to_file.h"

#define PIN 2 // 라즈베리파이 (13) `GPIO. 2`

// 조도센서에서 빛 유무를 받아오는 함수. 밝은 상태면 1, 어두운 상태면 0을 반환한다. 조도 센서가 밝음과 어두움을 구별하는 기준은 식물의 광합성 조건과 비슷하다.
int get_brightness()
{
    // LDR_PIN을 입력 모드로 설정
    pinMode(PIN, INPUT);
    int light_val = digitalRead(PIN);
    if (light_val == 1)
    {
        write_to_file_string("/tmp/brightness", "Bright");
    }
    else
    {
        write_to_file_string("/tmp/brightness", "Dark");
    }
    return light_val;
}
