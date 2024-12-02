# 샐러드 프로젝트 가이드라인

## 라이브러리

수업 시간에 배운 내용을 바탕으로 코드를 짜되, 수업시간에 배우지 않은 복잡한 센서/액츄에이터 코딩이 필요한 상황이 많습니다. 그때 우리는 [WiringPi](https://github.com/WiringPi/WiringPi)라는 라이브러리를 사용할 겁니다. [WiringPi 구현 예시 코드들](https://github.com/WiringPi/WiringPi/tree/master/examples)을 참고하여 코드를 짜 봅시다.

WiringPi로도 구현할 수 없는 부분들은 Python 등의 다른 언어를 사용해도 좋습니다.

### 컴파일 방법

```
gcc -o main main.c -l wiringPi
```

## 핀

### 할당된 핀 번호

### 핀 구조 빠르게 보는 법

`gpio readall`을 입력하면 핀 구조를 간단히 보여줌
