#include <wiringPi.h>
#include <stdio.h>
#include "DHT11.h"

#define MAXTIMINGS 100
#define DHTPIN 3 

// VCC -> 5V(No.4 PIN), DATA -> GPIO3(BCM : 22), GND -> GND(No.14 PIN)

// 해당 센서는 온습도센서로, 식물을 키우는 환경의 온도를 측정함.
// 실내 온도가 18도 미만 혹은 35도 초과일 경우 이벤트 발생
// C언어일 경우, DHTPIN 번호가 GPIO.n 즉, wPi와 같고, Python일 경우, BCM과 같다.

int dhtVal[5] = {0, 0, 0, 0, 0};

DHT11_Data readData()
{
     int laststate = HIGH;
     int counter = 0;
     int j = 0, i;
     DHT11_Data values = {0.0, 0.0}; 

     dhtVal[0] = dhtVal[1] = dhtVal[2] = dhtVal[3] = dhtVal[4] = 0;

     /* pull pin down for 18 milliseconds */
     pinMode(DHTPIN, OUTPUT);
     digitalWrite(DHTPIN, LOW);
     delay(18);

     /* then pull it up for 40 microseconds */
     digitalWrite(DHTPIN, HIGH);
     delayMicroseconds(40);

     /* prepare to read the pin */
     pinMode(DHTPIN, INPUT);

     /* detect change and read data */
     for (i = 0; i < MAXTIMINGS; i++)
     {
          counter = 0;
          while (digitalRead(DHTPIN) == laststate)
          {
               counter++;
               delayMicroseconds(1);
               if (counter == 255)
               {
                    break;
               }
          }

          laststate = digitalRead(DHTPIN);

          if (counter == 255)
               break;

          /* ignore first 3 transitions */
          if ((i >= 4) && (i % 2 == 0))
          {
               /* shove each bit into the storage bytes */
               dhtVal[j / 8] <<= 1;
               if (counter > 50)
                    dhtVal[j / 8] |= 1;
               j++;
          }
     }

     /*
      * check we read 40 bits (8bit x 5 ) + verify checksum in the last byte
      * print it out if data is good
      */
     if ((j >= 40) &&
         (dhtVal[4] == ((dhtVal[0] + dhtVal[1] + dhtVal[2] + dhtVal[3]) & 0xFF)))
     {
          values.humidity = dhtVal[0] + dhtVal[1] * 0.1;
          values.temperature = dhtVal[2] + dhtVal[3] * 0.1;
     }
     else
     {
          printf("Invalid Data! \n");
     }

     return values;
}

/*
int main(void)
{
     printf("Raspberry Pi wiringPi DHT11 Temperature test program\n");

     if (wiringPiSetup() == -1)
          return -1;

     while (1)
     {
          readData();
          delay(3000); 
     }

     return (0);
} 
*/