# 이 코드는 부팅 시 자동으로 실행됩니다.
# 자동으로 실행되지 않는다면 `crontab -e`로 자동실행 스크립트를 점검하세요.

from RPLCD.i2c import CharLCD
from subprocess import *
import socket
import threading
import time

PORT = 50000
BUFFER_SIZE = 16
IP_PLACEHOLDER = "Connecting..."
IP_ROW = 0  # IP 주소가 표시될 행
DATA_ROW = 1  # 클라이언트로부터 수신된 데이터가 표시될 행


# LCD 초기화
def init_lcd():
    lcd = CharLCD(
        i2c_expander="PCF8574", address=0x27, port=1, cols=16, rows=2, dotsize=8
    )
    lcd.cursor_mode = "hide"
    lcd.clear()
    return lcd


# 한 줄을 완전 초기화하고 텍스트를 출력
def lcd_write_row(row, text):
    if len(text) > BUFFER_SIZE:
        text = text[:BUFFER_SIZE]
    lcd.cursor_pos = (row, 0)
    lcd.write_string(" " * BUFFER_SIZE)
    lcd.cursor_pos = (row, 0)
    lcd.write_string(text)


# WiFi 인터페이스에 할당된 IP 주소를 구하는 명령어. wlan0이 WiFi에 연결되어 있지 않다면 IP_PLACEHOLDER을 반환.
def get_ip():
    ip_command = """
    ip -4 addr show wlan0 | grep -oP '(?<=inet\s)\d+(\.\d+){3}'
    """
    try:
        ip = check_output(ip_command, shell=True).decode("ascii")[:-1]
    except CalledProcessError:
        ip = IP_PLACEHOLDER
    finally:
        return ip


# LCD에서 IP 주소를 10초마다 갱신. row로 행 선택이 가능하고, interval을 조절하여 갱신 주기를 변경할 수 있음.
def lcd_ip_refresh(interval=10):
    while True:
        ip = get_ip()
        lcd_write_row(IP_ROW, ip)
        time.sleep(interval)


def handle_client(client_socket, addr):
    print(f"클라이언트 연결됨: {addr}")

    try:
        data = client_socket.recv(BUFFER_SIZE).decode()
        print(f"클라이언트 {addr}로부터 수신: {data}")
        lcd_write_row(DATA_ROW, data)
        print(f"클라이언트 {addr} 연결 종료")

    except ConnectionResetError:
        print(f"클라이언트 {addr} 연결이 강제로 종료됨")

    finally:
        client_socket.close()


def run_server():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    # 서버 주소 설정
    server_address = ("localhost", PORT)
    server.bind(server_address)
    server.listen(5)

    print(f"서버가 {server_address}에서 대기 중...")

    try:
        while True:
            # 클라이언트 연결 수락
            client_socket, addr = server.accept()
            # 각 클라이언트를 별도 스레드로 처리
            client_thread = threading.Thread(
                target=handle_client, args=(client_socket, addr)
            )
            client_thread.start()

    except KeyboardInterrupt:
        print("\n서버를 종료합니다.")

    finally:
        server.close()


if __name__ == "__main__":
    lcd = init_lcd()
    # IP 주소 갱신 스레드 시작. Daemon 스레드로 설정하여 메인 스레드 종료 시 함께 종료.
    ip_thread = threading.Thread(target=lcd_ip_refresh, daemon=True)
    ip_thread.start()
    run_server()
