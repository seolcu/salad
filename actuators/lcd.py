from RPLCD.i2c import CharLCD
from subprocess import check_output, CalledProcessError
import socket
import threading
import time
from typing import Optional
import logging
from dataclasses import dataclass

# 로깅 설정
logging.basicConfig(
    level=logging.INFO, format="%(asctime)s - %(levelname)s - %(message)s"
)


@dataclass
class LCDConfig:
    """LCD 설정을 위한 데이터 클래스"""

    port: int = 50000
    column_size: int = 16
    ip_placeholder: str = "Connecting..."
    i2c_address: int = 0x27
    lcd_port: int = 1
    lcd_rows: int = 2
    lcd_dotsize: int = 8


class LCDController:
    """LCD 제어 및 서버 관리를 위한 클래스"""

    def __init__(self, config: LCDConfig = LCDConfig()):
        self.config = config
        self.lcd = self._init_lcd()
        self.current_message: str = ""
        self.running: bool = False
        self._lock = threading.Lock()

    def _init_lcd(self) -> CharLCD:
        """LCD 초기화"""
        try:
            lcd = CharLCD(
                i2c_expander="PCF8574",
                address=self.config.i2c_address,
                port=self.config.lcd_port,
                cols=self.config.column_size,
                rows=self.config.lcd_rows,
                dotsize=self.config.lcd_dotsize,
            )
            lcd.cursor_mode = "hide"
            lcd.clear()
            return lcd
        except Exception as e:
            logging.error(f"LCD 초기화 실패: {e}")
            raise

    def get_ip(self) -> str:
        """WiFi IP 주소 조회"""
        cmd = "ip -4 addr show wlan0 | grep -oP '(?<=inet\s)\d+(\.\d+){3}'"
        try:
            return check_output(cmd, shell=True).decode("utf-8").strip()
        except CalledProcessError:
            logging.warning("IP 주소 조회 실패")
            return self.config.ip_placeholder
        except Exception as e:
            logging.error(f"예상치 못한 에러 발생: {e}")
            return self.config.ip_placeholder

    def update_message(self, message: str) -> None:
        """스레드 안전한 메시지 업데이트"""
        with self._lock:
            self.current_message = message[: self.config.column_size]

    def _handle_client(self, client_socket: socket.socket, addr: tuple) -> None:
        """클라이언트 연결 처리"""
        logging.info(f"클라이언트 연결됨: {addr}")
        try:
            data = client_socket.recv(self.config.column_size).decode("utf-8")
            if data:
                self.update_message(data)
                logging.info(f"수신된 메시지: {data}")
        except ConnectionResetError:
            logging.warning(f"클라이언트 {addr} 연결 강제 종료")
        except Exception as e:
            logging.error(f"클라이언트 처리 중 에러 발생: {e}")
        finally:
            client_socket.close()

    def _refresh_display(self) -> None:
        """LCD 디스플레이 갱신"""
        while self.running:
            try:
                with self._lock:
                    self.lcd.clear()
                    self.lcd.write_string(self.get_ip())
                    self.lcd.cursor_pos = (1, 0)
                    self.lcd.write_string(self.current_message)
                time.sleep(1)
            except Exception as e:
                logging.error(f"디스플레이 갱신 중 에러 발생: {e}")
                time.sleep(5)  # 에러 발생 시 잠시 대기

    def start(self) -> None:
        """서버 및 디스플레이 갱신 시작"""
        self.running = True

        # 디스플레이 갱신 스레드 시작
        display_thread = threading.Thread(target=self._refresh_display, daemon=True)
        display_thread.start()

        # 서버 시작
        server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

        try:
            server.bind(("localhost", self.config.port))
            server.listen(5)
            logging.info(f"서버가 포트 {self.config.port}에서 시작됨")

            while self.running:
                try:
                    client_socket, addr = server.accept()
                    client_thread = threading.Thread(
                        target=self._handle_client,
                        args=(client_socket, addr),
                        daemon=True,
                    )
                    client_thread.start()
                except Exception as e:
                    logging.error(f"클라이언트 연결 수락 중 에러 발생: {e}")

        except Exception as e:
            logging.error(f"서버 시작 중 에러 발생: {e}")
        finally:
            self.running = False
            server.close()
            self.lcd.clear()
            logging.info("서버가 종료됨")


if __name__ == "__main__":
    try:
        controller = LCDController()
        controller.start()
    except KeyboardInterrupt:
        logging.info("프로그램이 사용자에 의해 종료됨")
    except Exception as e:
        logging.error(f"프로그램 실행 중 치명적인 에러 발생: {e}")
