import socket
from playsound3 import playsound
from gtts import gTTS
import os
import json


class TTSServer:
    def __init__(self, host="localhost", port=50001):
        """음성 출력 서버를 초기화합니다."""
        # 소켓 서버 초기화
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.server_socket.bind((host, port))
        self.server_socket.listen(1)

        # 음성 파일 카운터 초기화
        self.audio_counter = 0

        print(f"TTS 서버가 {host}:{port}에서 시작되었습니다.")

    def play_audio(self, text):
        """텍스트를 음성으로 변환하여 재생합니다."""
        # 고유한 파일 이름 생성
        filename = f"response_{self.audio_counter}.mp3"
        self.audio_counter += 1

        try:
            # 텍스트를 음성으로 변환
            tts = gTTS(text=text, lang="ko")
            tts.save(filename)

            # 음성 파일 재생
            playsound(filename)
            return True

        except Exception as e:
            print(f"오디오 재생 중 오류 발생: {e}")
            return False

        finally:
            # 임시 파일 삭제
            if os.path.exists(filename):
                try:
                    os.remove(filename)
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


if __name__ == "__main__":
    server = TTSServer()
    server.run()
