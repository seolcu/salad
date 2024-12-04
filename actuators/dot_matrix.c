#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdio.h>

#define SPI_CHANNEL 0
#define SPI_SPEED 1000000

//도트 매트릭스에 나올 패턴들
unsigned char smilePattern[8] = {
    0b00111100,
    0b01000010,
    0b10100101,
    0b10000001,
    0b10100101,
    0b10011001,
    0b01000010,
    0b00111100
};
unsigned char sadPattern[8] = {
    0b01111110,
    0b10000001,
    0b10100101,
    0b10000001,
    0b10011001,
    0b10100101,
    0b10000001,
    0b01111110
};
unsigned char MuPyoJeongPattern[8] = {
    0b00111100,
    0b01000010,
    0b10100101,
    0b10000001,
    0b10000001,
    0b10111101,
    0b01000010,
    0b00111100
};
unsigned char KimPattern[8] = {
    0b11111001,
    0b00001001,
    0b00001001,
    0b00001001,
    0b00000000,
    0b11111111,
    0b10000001,
    0b11111111
};
unsigned char YeongPattern[8] = {
    0b01100001,
    0b10010111,
    0b10010001,
    0b01100111,
    0b00000001,
    0b01111110,
    0b10000001,
    0b01111110
};
unsigned char MinPattern[8] = {
    0b11111001,
    0b10001001,
    0b10001001,
    0b11111001,
    0b00000000,
    0b10000000,
    0b10000000,
    0b11111111
};

void setup() {
    wiringPiSetup();
    wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED);
}

//한 줄씩 출력할 명령을 wiringPi통해 보냄
void sendCommand(unsigned char address, unsigned char data) {
    unsigned char buffer[2];
    buffer[0] = address;
    buffer[1] = data;
    wiringPiSPIDataRW(SPI_CHANNEL, buffer, 2);
}

void setLED(int row, int col, int value) {
    unsigned char buffer[2];
    buffer[0] = row + 1;
    buffer[1] = value ? (1 << col) : 0;
    wiringPiSPIDataRW(SPI_CHANNEL, buffer, 2);
}

//각 패턴을 출력하는 함수들
void Clear() {
    for (int row = 0; row < 8; row++) {
        sendCommand(row + 1, 0b00000000);
    }
}
void drawSmile() {
    for (int row = 0; row < 8; row++) {
        sendCommand(row + 1, smilePattern[row]);
    }
}
void drawSad() {
    for (int row = 0; row < 8; row++) {
        sendCommand(row + 1, sadPattern[row]);
    }
}
void drawMuPyoJeong() {
    for (int row = 0; row < 8; row++) {
        sendCommand(row + 1, MuPyoJeongPattern[row]);
    }
}
void drawKim() {
    for (int row = 0; row < 8; row++) {
        sendCommand(row + 1, KimPattern[row]);
    }
}
void drawYeong() {
    for (int row = 0; row < 8; row++) {
        sendCommand(row + 1, YeongPattern[row]);
    }
}
void drawMin() {
    for (int row = 0; row < 8; row++) {
        sendCommand(row + 1, MinPattern[row]);
    }
}

int main() {
    setup();

    // 매트릭스 초기화 명령어
    sendCommand(0x09, 0x00); // Decode mode: none
    sendCommand(0x0A, 0x03); // Intensity: medium
    sendCommand(0x0B, 0x07); // Scan limit: all rows
    sendCommand(0x0C, 0x01); // Shutdown register: normal operation
    sendCommand(0x0F, 0x00); // Display test: off

    //도트 매트릭스에 화면을 그리는 코드 추가하는 부분
    //아래 두 명령은 그것의 예시
    Clear();
    drawSmile();

    return 0;
}