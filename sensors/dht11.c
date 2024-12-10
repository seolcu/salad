#include <wiringPi.h>
#include <stdio.h>
#include "dht11.h"

#define MAXTIMINGS 100
#define PIN 3 // 라즈베리파이 (15) `GPIO. 3` / `BCM. 22`

// 해당 센서는 DHT11 온습도센서로, 식물을 키우는 환경의 온도를 측정함.
// 실내 온도가 18도 미만 혹은 35도 초과일 경우 이벤트 발생
// C언어일 경우, PIN 번호가 GPIO.n 즉, wPi와 같고, Python일 경우, BCM과 같다.

int dht_val[5] = {0, 0, 0, 0, 0}; // DHT11 온습도센서를 통해 읽어온 데이터를 저장하는 배열

// 측정 시 오류가 발생했을 사용하기 위한 변수

// DHT11 온습도 센서에서 온도값을 읽어오는 함수.
float get_temperature()
{
     int last_state = HIGH;
     int counter = 0;
     int j = 0, i;
     float temperature;

     while (1)
     {
          dht_val[0] = dht_val[1] = dht_val[2] = dht_val[3] = dht_val[4] = 0;
          // dht_val[0] : 습도 정수부, dht_val[1] : 습도 소수부, dht_val[2] : 온도 정수부, dht_val[3] : 온도 소수부, dht_val[4] : checksum
          // 해당 프로젝트에서는 습도를 DHT11 온습도센서에서 측정된 습도가 아닌 토양습도센서로 측정된 습도값을 사용하므로 dht_val[0],[1]의 값은 사용하지 않는다.

          // DHT11 온습도센서 초기화 단계 - 라즈베리파이의 PIN을 센서로부터 값을 받아올 수 있게 하기 위한 과정
          pinMode(PIN, OUTPUT);
          digitalWrite(PIN, LOW);
          delay(18);

          digitalWrite(PIN, HIGH);
          delayMicroseconds(40);

          pinMode(PIN, INPUT); // 핀 모드를 "INPUT" 입력 상태로 전환하여 센서로부터 데이터를 받을 준비를 한다.

          // 신호 변화 감지 및 데이터 저장 단계
          for (i = 0; i < MAXTIMINGS; i++)
          {
               counter = 0;
               while (digitalRead(PIN) == last_state)
               {
                    counter++;
                    delayMicroseconds(1);
                    if (counter == 255)
                    {
                         break;
                    }
               }

               last_state = digitalRead(PIN);

               if (counter == 255)
                    break;

               if ((i >= 4) && (i % 2 == 0))
               {
                    dht_val[j / 8] <<= 1;
                    if (counter > 50)
                         dht_val[j / 8] |= 1;
                    j++;
               }
          }

          // 유효성 검사 단계 - 읽은 데이터가 총 40비트 이상인지 확인하고 마지막 바이트(dht_val[4])가 체크섬 계산 결과와 일치하는지 확인한다.
          if ((j >= 40) && (dht_val[4] == ((dht_val[0] + dht_val[1] + dht_val[2] + dht_val[3]) & 0xFF)))
          {
               temperature = dht_val[2] + dht_val[3] * 0.1; // 측정한 온도 데이터 dht_val[3], dht_val[4]를 계산하여 float형의 온도값 데이터를 계산한다.
               // 온도값을 파일로 저장
               return temperature;
          }

          else // 체크섬 계산 결과와 일치하지 않거나 읽은 데이터가 정상적이지 않은 경우, 다시 실행하여 정상 측정값을 도출한다.
          {
               j = 0;
               delay(500);
          }
     }
}
