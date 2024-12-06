#include <wiringPi.h>
#include <stdio.h>
#include "dht11.h"

#define MAXTIMINGS 100
#define PIN 3 // 라즈베리파이 (15) `GPIO. 3` / `BCM. 22`

// 해당 센서는 온습도센서로, 식물을 키우는 환경의 온도를 측정함.
// 실내 온도가 18도 미만 혹은 35도 초과일 경우 이벤트 발생
// C언어일 경우, PIN 번호가 GPIO.n 즉, wPi와 같고, Python일 경우, BCM과 같다.

int dht_val[5] = {0, 0, 0, 0, 0};

// DHT11 온습도 센서에서 온도값을 읽어오는 함수.
float get_temperature()
{
     int last_state = HIGH;
     int counter = 0;
     int j = 0, i;
     float dht11_temp = -1; // 온습도센서가 측정할 온도 변수, 초기화는 -1로 하여 오류 시 반환할 값으로 설정해놓는다.

     dht_val[0] = dht_val[1] = dht_val[2] = dht_val[3] = dht_val[4] = 0;

     /* pull pin down for 18 milliseconds */
     pinMode(PIN, OUTPUT);
     digitalWrite(PIN, LOW);
     delay(18);

     /* then pull it up for 40 microseconds */
     digitalWrite(PIN, HIGH);
     delayMicroseconds(40);

     /* prepare to read the pin */
     pinMode(PIN, INPUT);

     /* detect change and read data */
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

          /* ignore first 3 transitions */
          if ((i >= 4) && (i % 2 == 0))
          {
               /* shove each bit into the storage bytes */
               dht_val[j / 8] <<= 1;
               if (counter > 50)
                    dht_val[j / 8] |= 1;
               j++;
          }
     }

     /*
      * check we read 40 bits (8bit x 5 ) + verify checksum in the last byte
      * print it out if data is good
      */
     if ((j >= 40) &&
         (dht_val[4] == ((dht_val[0] + dht_val[1] + dht_val[2] + dht_val[3]) & 0xFF)))
     {
          dht11_temp = dht_val[2] + dht_val[3] * 0.1;
     }
     else
     {
          printf("Invalid Data! \n");
     }

     return dht11_temp;
}

/*
int main(void)
{
     printf("Raspberry Pi wiringPi DHT11 Temperature test program\n");

     if (wiringPiSetup() == -1)
          return -1;

     while (1)
     {
          get_temperature();
          delay(3000);
     }

     return (0);
}
*/