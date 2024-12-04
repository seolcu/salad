#include <wiringPi.h>
#include <stdio.h>

#define MAXTIMINGS 100
#define DHTPIN 3 // VCC -> 5V(No.4 PIN), DATA -> GPIO4(No.7 PIN), GND -> GND(No.14 PIN)

int dhtVal[5] = {0, 0, 0, 0, 0};

void readData()
{
     int laststate = HIGH;
     int counter = 0;
     int j = 0, i;
     float f; /* fahrenheit */

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
          f = dhtVal[2] * 9. / 5. + 32;
          printf("Humidity = %d.%d %% Temperature = %d.%d *C (%.1f *F)\n",
                 dhtVal[0], dhtVal[1], dhtVal[2], dhtVal[3], f);
     }
     else
     {
          printf("Invalid Data! \n");
     }
     return;
}

int main(void)
{
     printf("Raspberry Pi wiringPi DHT11 Temperature test program\n");

     if (wiringPiSetup() == -1)
          return -1;

     while (1)
     {
          readData();
          delay(3000); /* wait 3sec to refresh */
     }

     return (0);
}