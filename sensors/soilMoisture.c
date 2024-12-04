#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

#define SPI_CHANNEL 1
#define SPI_SPEED 1000000
#define CS_PIN 7 // 파이의 CS1번 핀. CS0은 도트매트릭스 연결됨

int readMCP3008(int channel)
{
    unsigned char buffer[3] = {1, (8 + channel) << 4, 0};
    digitalWrite(CS_PIN, LOW);
    wiringPiSPIDataRW(SPI_CHANNEL, buffer, 3);
    digitalWrite(CS_PIN, HIGH); // 이 함수의 digitalWrite LOW HIGH는 CS1번 핀을 사용하기 위해 활성화/비활성화를 해 주는 과정이다.
    return ((buffer[1] & 3) << 8) + buffer[2];
}

int main()
{
    if (wiringPiSetup() == -1)
    {
        printf("WiringPi 초기화 실패\n");
        return 1;
    }

    if (wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) == -1)
    {
        printf("SPI 초기화 실패\n");
        return 1;
    }

    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);

    while (1)
    {
        int value = readMCP3008(0);
        float moisture = (float)value / 1023.0 * 100.0;
        printf("토양 습도: %.2f%%\n", moisture);
        delay(1000);
    }

    return 0;
}