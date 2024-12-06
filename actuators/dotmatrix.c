#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdio.h>

#define SPI_CHANNEL 0
#define SPI_SPEED 1000000

// 도트 매트릭스에 나올 패턴들
unsigned char smile_pattern[8] = {
    0b00111100,
    0b01000010,
    0b10100101,
    0b10000001,
    0b10100101,
    0b10011001,
    0b01000010,
    0b00111100};
unsigned char sad_pattern[8] = {
    0b00111100,
    0b01000010,
    0b10100101,
    0b10000001,
    0b10011001,
    0b10100101,
    0b01000010,
    0b00111100};
unsigned char neutral_pattern[8] = {
    0b00111100,
    0b01000010,
    0b10100101,
    0b10000001,
    0b10000001,
    0b10111101,
    0b01000010,
    0b00111100};
unsigned char kim_pattern[8] = {
    0b11111001,
    0b00001001,
    0b00001001,
    0b00001001,
    0b00000000,
    0b11111111,
    0b10000001,
    0b11111111};
unsigned char yeong_pattern[8] = {
    0b01100001,
    0b10010111,
    0b10010001,
    0b01100111,
    0b00000001,
    0b01111110,
    0b10000001,
    0b01111110};
unsigned char min_pattern[8] = {
    0b11111001,
    0b10001001,
    0b10001001,
    0b11111001,
    0b00000000,
    0b10000000,
    0b10000000,
    0b11111111};

void setup()
{
    wiringPiSetup();
    wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED);
}

// 한 줄씩 출력할 명령을 wiringPi통해 보냄
void send_command(unsigned char address, unsigned char data)
{
    unsigned char buffer[2];
    buffer[0] = address;
    buffer[1] = data;
    wiringPiSPIDataRW(SPI_CHANNEL, buffer, 2);
}

void setLED(int row, int col, int value)
{
    unsigned char buffer[2];
    buffer[0] = row + 1;
    buffer[1] = value ? (1 << col) : 0;
    wiringPiSPIDataRW(SPI_CHANNEL, buffer, 2);
}

// 각 패턴을 출력하는 함수들
void clear()
{
    for (int row = 0; row < 8; row++)
    {
        send_command(row + 1, 0b00000000);
    }
}
void draw_smile()
{
    for (int row = 0; row < 8; row++)
    {
        send_command(row + 1, smile_pattern[row]);
    }
}
void draw_sad()
{
    for (int row = 0; row < 8; row++)
    {
        send_command(row + 1, sad_pattern[row]);
    }
}
void draw_neutral()
{
    for (int row = 0; row < 8; row++)
    {
        send_command(row + 1, neutral_pattern[row]);
    }
}
void draw_kim()
{
    for (int row = 0; row < 8; row++)
    {
        send_command(row + 1, kim_pattern[row]);
    }
}
void draw_yeong()
{
    for (int row = 0; row < 8; row++)
    {
        send_command(row + 1, yeong_pattern[row]);
    }
}
void draw_min()
{
    for (int row = 0; row < 8; row++)
    {
        send_command(row + 1, min_pattern[row]);
    }
}

int main()
{
    setup();

    // 매트릭스 초기화 명령어
    send_command(0x09, 0x00); // Decode mode: none
    send_command(0x0A, 0x03); // Intensity: medium
    send_command(0x0B, 0x07); // Scan limit: all rows
    send_command(0x0C, 0x01); // Shutdown register: normal operation
    send_command(0x0F, 0x00); // Display test: off

    // 도트 매트릭스에 화면을 그리는 코드 추가하는 부분
    // 아래 두 명령은 그것의 예시
    clear();
    draw_smile();
    delay(1000);
    draw_sad();
    delay(1000);
    draw_kim();
    delay(1000);
    draw_yeong();
    delay(1000);
    draw_min();
    delay(1000);
    draw_neutral();

    return 0;
}