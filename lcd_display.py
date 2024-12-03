# 이 코드는 부팅될때마다 자동으로 실행됩니다.
# 자동으로 실행되지 않는다면 `crontab -e`로 자동실행 스크립트를 점검하세요.

from RPLCD.i2c import CharLCD
from subprocess import *

lcd = CharLCD(i2c_expander="PCF8574", address=0x27, port=1, cols=16, rows=2, dotsize=8)
lcd.clear()

# WiFi 인터페이스에 할당된 IP 주소를 구하는 명령어
ip_command = """
ip -4 addr show wlan0 | grep -oP '(?<=inet\s)\d+(\.\d+){3}'
"""

ip = check_output(ip_command, shell=True).decode("ascii")[:-1]

lcd.write_string(ip)
