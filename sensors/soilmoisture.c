#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include "soilmoisture.h"
#include "../utility/write_to_file.h"

#define SPI_CHANNEL 1
#define SPI_SPEED 1000000
#define MCP3008_CHANNEL 0
#define CS_PIN 11 // 라즈베리파이 (26) `CE1` / `wPi. 11`

// 토양습도 센서의 값을 읽어오는 함수. % (퍼센트) 단위로 반환한다.
float get_soilmoisture()
{
    if (wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) == -1)
    {
        printf("SPI 초기화 실패\n");
        return 1;
    }

    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);

    unsigned char buffer[3] = {1, (8 + MCP3008_CHANNEL) << 4, 0};
    digitalWrite(CS_PIN, LOW);
    wiringPiSPIDataRW(SPI_CHANNEL, buffer, 3);
    digitalWrite(CS_PIN, HIGH); // 이 함수의 digitalWrite LOW HIGH는 CS1번 핀을 사용하기 위해 활성화/비활성화를 해 주는 과정이다.
    // data 값은 0 ~ 1023 사이의 값이다. 0에 가까울수록 습도가 높다.
    int data = ((buffer[1] & 3) << 8) + buffer[2];
    // 습도를 의미하는 값이므로, moisture 값은 data의 값이 작아질수록 커진다.
    float moisture = 100.0 - (float)data / 1023.0 * 100.0;
    write_to_file_float("/tmp/soilmoisture", "%.1f", moisture);
    return moisture;
}