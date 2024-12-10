import socket
import speech_recognition as sr
import anthropic
import os
from dotenv import load_dotenv
import time
import json

DELAY_AFTER_CONVERSATION = 60  # 대화 종료 후 대기 시간 (초)


class InteractivePlant:
    def __init__(self):
        # 환경 변수 설정
        load_dotenv()

        # 모션 감지를 위한 소켓 서버 초기화 (포트 50002)
        self.motion_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.motion_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.motion_socket.bind(("localhost", 50002))
        self.motion_socket.listen(1)
        print("모션 감지 서버가 포트 50002에서 시작되었습니다...")

        # TTS 서버 접속 정보
        self.tts_host = "localhost"
        self.tts_port = 50001

        # 음성 인식 시스템 초기화
        self.recognizer = sr.Recognizer()
        self.microphone = sr.Microphone()

        # Claude AI 시스템 초기화
        self.claude_client = anthropic.Client(api_key=os.getenv("ANTHROPIC_API_KEY"))

        # 대화 기록 관리
        self.conversation_history = []

        # 종료 명령어 설정
        self.exit_commands = [
            "안녕히 계세요",
            "잘 있어요",
            "종료",
            "끝",
            "그만",
            "바이",
            "그만할게",
            "그만하자",
        ]

    def send_to_tts_server(self, text):
        """TTS 서버에 텍스트를 전송하여 음성으로 출력합니다."""
        try:
            # TTS 서버에 연결
            tts_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            tts_socket.connect((self.tts_host, self.tts_port))

            # 메시지 전송
            message = json.dumps({"text": text})
            tts_socket.send(message.encode("utf-8"))

            # 응답 수신
            response = tts_socket.recv(1024).decode("utf-8")
            response_data = json.loads(response)

            return response_data.get("success", False)

        except Exception as e:
            print(f"TTS 서버 통신 오류: {e}")
            return False

        finally:
            tts_socket.close()

    def wait_for_motion_signal(self):
        """모션 감지 신호를 기다립니다."""
        print("\n모션 감지 신호 대기 중...")

        # 새로운 연결을 기다리기 전에 이전 연결이 있다면 모두 처리
        self.motion_socket.setblocking(False)
        while True:
            try:
                temp_socket, _ = self.motion_socket.accept()
                temp_socket.close()
            except BlockingIOError:
                break
            except socket.error:
                break

        # 다시 블로킹 모드로 설정하고 새로운 연결 대기
        self.motion_socket.setblocking(True)
        client_socket, addr = self.motion_socket.accept()

        try:
            data = client_socket.recv(1024).decode("utf-8")
            if data == "MOTION_DETECTED":
                print("모션이 감지되었습니다!")
                return True
        finally:
            client_socket.close()

        return False

    def listen_to_speech(self):
        """사용자의 음성을 텍스트로 변환합니다."""
        with self.microphone as source:
            print("듣고 있습니다...")
            self.recognizer.adjust_for_ambient_noise(source)
            audio = self.recognizer.listen(source)

        try:
            text = self.recognizer.recognize_google(audio, language="ko-KR")
            print(f"인식된 텍스트: {text}")
            return text
        except sr.UnknownValueError:
            print("음성을 인식할 수 없습니다")
            return None
        except sr.RequestError as e:
            print(f"음성 인식 서비스 오류: {e}")
            return None

    def get_claude_response(self, user_input):
        """Claude AI로부터 응답을 생성합니다."""
        try:
            system_prompt = (
                "당신은 식물입니다. 사람들과 짧고 친근하게 대화하세요. "
                "2-3문장 정도의 간단한 답변을 하되, 식물의 관점에서 자연스럽게 이야기하세요. "
                "행동이나 동작을 묘사하지 말고, 순수하게 대화 내용만 전달하세요."
                "이미 첫 인사를 한 상황이므로, 새롭게 인사하지 마세요"
            )

            messages = []
            for msg in self.conversation_history[-5:]:
                messages.append(msg)

            messages.append({"role": "user", "content": user_input})

            response = self.claude_client.messages.create(
                model="claude-3-5-sonnet-latest",
                max_tokens=1000,
                system=system_prompt,
                messages=messages,
            )

            response_text = response.content[0].text
            self.conversation_history.append({"role": "user", "content": user_input})
            self.conversation_history.append(
                {"role": "assistant", "content": response_text}
            )

            return response_text

        except Exception as e:
            print(f"Claude API 오류: {e}")
            return "죄송합니다. 잠시 후에 다시 말씀해 주시겠어요?"

    def run(self):
        """메인 실행 함수입니다."""
        print("시스템을 시작합니다...")

        try:
            while True:  # 외부 루프: 모션 감지 순환
                # 모션 감지 신호를 기다립니다
                if self.wait_for_motion_signal():
                    # 대화 시작
                    welcome_message = "안녕하세요! 저는 당신과 이야기 나누고 싶은 식물이에요. 편하게 이야기해주세요."
                    print("모션 감지: 인사말 재생")
                    self.send_to_tts_server(welcome_message)

                    # 내부 루프: 대화 진행
                    while True:
                        user_input = self.listen_to_speech()
                        if not user_input:
                            continue

                        # 종료 조건 확인
                        if any(command in user_input for command in self.exit_commands):
                            farewell = "즐거운 대화였어요. 다음에 또 이야기해요!"
                            self.send_to_tts_server(farewell)
                            # 대화 기록 초기화
                            self.conversation_history = []
                            # 대화가 끝나자마자 말 걸지 않게, 대기 시간을 둠
                            time.sleep(DELAY_AFTER_CONVERSATION)
                            break  # 내부 루프를 종료하고 모션 감지 대기 상태로 복귀

                        # 응답 생성 및 재생
                        response = self.get_claude_response(user_input)
                        print(f"Claude 응답: {response}")
                        self.send_to_tts_server(response)

                        time.sleep(0.5)

        except KeyboardInterrupt:
            print("\n프로그램을 종료합니다.")
        except Exception as e:
            print(f"예상치 못한 오류 발생: {e}")
        finally:
            self.motion_socket.close()
            print("소켓 연결이 종료되었습니다.")


if __name__ == "__main__":
    # .env 파일에 ANTHROPIC_API_KEY가 설정되어 있어야 합니다
    plant = InteractivePlant()
    plant.run()
