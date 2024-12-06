#include <wiringPi.h>
#include <stdio.h>
#ifndef DHT11_H
#define DHT11_H

typedef struct {
    float humidity;    // 습도 값
    float temperature; // 온도 값 (섭씨 기준)
} DHT11_Data;

DHT11_Data readData(); // DHT11 온습도센서에서 측정한 실내 온도값 및 습도값을 return해줌.

#endif