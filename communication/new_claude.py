import speech_recognition as sr
from gtts import gTTS
import pygame
import anthropic
import os
from dotenv import load_dotenv
import time


class InteractivePlant:
    def __init__(self):
        # .env 파일에서 환경 변수를 로드합니다.
        # 이는 API 키와 같은 민감한 정보를 코드에서 분리하여 관리하기 위한 좋은 방법입니다.
        load_dotenv()

        # 음성 인식 시스템을 초기화합니다.
        # Recognizer는 음성을 텍스트로 변환하는 주요 컴포넌트입니다.
        self.recognizer = sr.Recognizer()
        self.microphone = sr.Microphone()

        # Claude AI 시스템을 초기화합니다.
        # api_key 매개변수를 사용하여 클라이언트를 설정합니다.
        self.claude_client = anthropic.Client(api_key=os.getenv("ANTHROPIC_API_KEY"))

        # pygame 오디오 시스템을 초기화합니다.
        # 여기서는 특별히 드라이버를 지정하여 JACK 오류를 방지합니다.
        try:
            pygame.mixer.init(
                frequency=44100,  # 표준 샘플링 주파수
                size=-16,  # 16비트 오디오, 부호 있음
                channels=2,  # 스테레오
                buffer=2048,  # 오디오 버퍼 크기
            )
        except pygame.error as e:
            print(f"오디오 시스템 초기화 중 오류 발생: {e}")
            print("기본 설정으로 다시 시도합니다...")
            pygame.mixer.init()

        # 대화 기록을 저장할 리스트를 초기화합니다.
        # 이는 자연스러운 대화 흐름을 위해 이전 대화 내용을 추적합니다.
        self.conversation_history = []

    def play_audio(self, text):
        """텍스트를 음성으로 변환하여 재생하는 함수입니다.

        이 함수는 다음과 같은 단계로 작동합니다:
        1. 주어진 텍스트를 Google TTS를 사용하여 음성 파일로 변환
        2. 생성된 음성 파일을 pygame을 통해 재생
        3. 재생이 완료되면 임시 파일을 삭제

        Args:
            text (str): 음성으로 변환할 텍스트
        """
        try:
            # gTTS를 사용하여 텍스트를 한국어 음성으로 변환합니다
            tts = gTTS(text=text, lang="ko")
            tts.save("response.mp3")

            # pygame으로 음성 파일을 재생합니다
            pygame.mixer.music.load("response.mp3")
            pygame.mixer.music.play()

            # 재생이 완료될 때까지 대기합니다
            while pygame.mixer.music.get_busy():
                pygame.time.wait(100)

        except Exception as e:
            print(f"오디오 재생 중 오류 발생: {e}")
        finally:
            # 재생 후 임시 파일을 삭제합니다
            if os.path.exists("response.mp3"):
                try:
                    os.remove("response.mp3")
                except:
                    pass

    def listen_to_speech(self):
        """사용자의 음성을 받아 텍스트로 변환하는 함수

        Returns:
            str or None: 변환된 텍스트 또는 인식 실패시 None

        이 함수는 마이크를 통해 음성을 입력받고, Google의 음성 인식 서비스를 사용하여
        텍스트로 변환합니다. 주변 소음을 고려하여 마이크 감도를 자동으로 조정합니다.
        """
        with self.microphone as source:
            print("듣고 있습니다...")
            # 주변 소음을 고려하여 마이크 감도 자동 조정
            self.recognizer.adjust_for_ambient_noise(source)
            audio = self.recognizer.listen(source)

        try:
            # Google Speech Recognition으로 음성을 한국어 텍스트로 변환
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
        """사용자 입력에 대한 Claude AI의 응답을 생성하는 함수입니다.

        이 함수는 다음과 같은 과정으로 작동합니다:
        1. 시스템 프롬프트를 별도의 매개변수로 설정
        2. 이전 대화 기록과 현재 사용자 입력을 포함한 메시지 배열 구성
        3. Claude API를 호출하여 응답 생성
        4. 응답을 처리하고 대화 기록 업데이트
        """
        try:
            # 시스템 프롬프트를 별도로 설정
            system_prompt = (
                "당신은 식물입니다. 사람들과 짧고 친근하게 대화하세요. "
                "2-3문장 정도의 간단한 답변을 하되, 식물의 관점에서 자연스럽게 이야기하세요. "
                "행동이나 동작을 묘사하지 말고, 순수하게 대화 내용만 전달하세요. "
                "예를 들어 '잎사귀를 흔들며' 같은 행동 묘사는 하지 않습니다."
            )

            # 대화 메시지 배열 구성 - 시스템 메시지 제외
            messages = []

            # 최근 5개의 대화 기록만 포함
            for msg in self.conversation_history[-5:]:
                messages.append(msg)

            # 현재 사용자 입력 추가
            messages.append({"role": "user", "content": user_input})

            # Claude API 호출 - system을 별도 매개변수로 전달
            response = self.claude_client.messages.create(
                model="claude-3-sonnet-20240229",
                max_tokens=1000,
                system=system_prompt,  # 시스템 프롬프트를 별도 매개변수로 전달
                messages=messages,
            )

            # 응답 처리 및 대화 기록 업데이트
            response_text = response.content[0].text
            self.conversation_history.append({"role": "user", "content": user_input})
            self.conversation_history.append(
                {"role": "assistant", "content": response_text}
            )

            return response_text

        except Exception as e:
            print(f"Claude API 오류: {e}")
            return "죄송합니다. 잠시 대화를 이어가기 어려운 상황이에요. 잠시 후에 다시 말씀해 주시겠어요?"

    def run(self):
        """대화 시스템의 메인 실행 함수입니다.

        이 함수는 사용자와의 대화를 관리하며, 다음과 같은 특징을 가집니다:
        1. 초기 인사로 대화를 시작합니다
        2. 사용자의 음성 입력을 지속적으로 받습니다
        3. 다양한 종료 명령을 인식하여 자연스럽게 대화를 종료합니다
        4. 각 응답에 대해 음성 출력을 제공합니다
        """
        print("AI 대화 시스템을 시작합니다...")

        try:
            # 초기 인사 재생
            welcome_message = "안녕하세요! 저는 당신과 이야기 나누고 싶은 식물이에요. 편하게 이야기해주세요."
            print("시작: 인사말 재생")
            self.play_audio(welcome_message)

            # 종료 명령어 목록을 설정합니다
            exit_commands = [
                "안녕히 계세요",
                "잘 있어요",
                "종료",
                "끝",
                "그만",
                "바이",
                "안녕",
                "그만할게",
                "그만하자",
            ]

            # 메인 대화 루프
            while True:
                # 사용자 음성 입력 받기
                user_input = self.listen_to_speech()
                if not user_input:
                    continue

                # 종료 조건 체크 - 입력된 문장에 종료 명령어가 포함되어 있는지 확인
                if any(command in user_input for command in exit_commands):
                    farewell = "즐거운 대화였어요. 다음에 또 이야기해요!"
                    self.play_audio(farewell)
                    break

                # Claude AI 응답 생성 및 재생
                response = self.get_claude_response(user_input)
                print(f"Claude 응답: {response}")
                self.play_audio(response)

                # 다음 대화를 위한 준비 시간
                time.sleep(0.5)

        except KeyboardInterrupt:
            print("\n프로그램을 종료합니다.")
        except Exception as e:
            print(f"예상치 못한 오류 발생: {e}")


if __name__ == "__main__":
    # 시스템 실행을 위해서는 .env 파일에 다음과 같이 API 키가 설정되어 있어야 합니다:
    # ANTHROPIC_API_KEY=your-api-key-here

    plant = InteractivePlant()
    plant.run()
