import time
import adafruit_dht
import threading
import os
import glob
from board import *


# GPIO4
SENSOR_PIN = D4

dht11 = adafruit_dht.DHT11(SENSOR_PIN, use_pulseio=False)
temperature = 0
humidity = 0


def get_dht():
    global temperature
    global humidity

    while True:
        try:
            temperature = dht11.temperature
            humidity = dht11.humidity
            print(f"Humidity= {humidity:.2f}")
            print(f"Temperature= {temperature:.2f}°C")
        except RuntimeError:
            print("Failed")
        time.sleep(0.5)


dht_thread = threading.Thread(target=get_dht)
# server_thread=threading.Thread(target=server)
dht_thread.daemon = True
# server_thread.daemon=True
# server_thread.start()
dht_thread.start()
# server_thread.join()
dht_thread.join()
print("end...")
