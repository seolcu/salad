import socket
import pygame
from gtts import gTTS
import os
import json


class TTSServer:
    def __init__(self, host="localhost", port=50001):
        """음성 출력 서버를 초기화합니다.

        이 서버는 네트워크를 통해 텍스트를 받아 음성으로 변환하여 출력합니다.
        """
        # 소켓 서버 초기화
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.server_socket.bind((host, port))
        self.server_socket.listen(1)

        # pygame 오디오 시스템 초기화
        try:
            pygame.mixer.init(frequency=44100, size=-16, channels=2, buffer=2048)
        except pygame.error as e:
            print(f"오디오 시스템 초기화 중 오류 발생: {e}")
            print("기본 설정으로 다시 시도합니다...")
            pygame.mixer.init()

        print(f"TTS 서버가 {host}:{port}에서 시작되었습니다.")

    def play_audio(self, text):
        """텍스트를 음성으로 변환하여 재생합니다."""
        try:
            # 텍스트를 음성으로 변환
            tts = gTTS(text=text, lang="ko")
            tts.save("response.mp3")

            # 음성 파일 재생
            pygame.mixer.music.load("response.mp3")
            pygame.mixer.music.play()

            # 재생이 완료될 때까지 대기
            while pygame.mixer.music.get_busy():
                pygame.time.wait(100)

            return True

        except Exception as e:
            print(f"오디오 재생 중 오류 발생: {e}")
            return False

        finally:
            # 임시 파일 삭제
            if os.path.exists("response.mp3"):
                try:
                    os.remove("response.mp3")
                except:
                    pass

    def run(self):
        """서버의 메인 실행 루프입니다."""
        try:
            while True:
                # 클라이언트 연결 대기
                print("\n텍스트 입력 대기 중...")
                client_socket, addr = self.server_socket.accept()

                try:
                    # 데이터 수신
                    data = client_socket.recv(4096).decode("utf-8")
                    if not data:
                        continue

                    # JSON 디코딩
                    try:
                        message = json.loads(data)
                        text = message.get("text", "")
                        if text:
                            print(f"받은 텍스트: {text}")
                            success = self.play_audio(text)
                            # 결과 전송
                            response = json.dumps(
                                {
                                    "success": success,
                                    "message": "재생 완료" if success else "재생 실패",
                                }
                            )
                            client_socket.send(response.encode("utf-8"))
                    except json.JSONDecodeError:
                        print("잘못된 JSON 형식")
                        response = json.dumps(
                            {"success": False, "message": "잘못된 JSON 형식"}
                        )
                        client_socket.send(response.encode("utf-8"))

                finally:
                    client_socket.close()

        except KeyboardInterrupt:
            print("\n서버를 종료합니다.")
        finally:
            self.server_socket.close()
            pygame.mixer.quit()


if __name__ == "__main__":
    server = TTSServer()
    server.run()
