# 샐러드 프로젝트 가이드라인

## 사용하는 라이브러리

최대한 수업 시간에 배운 내용을 바탕으로 코드를 짜되, 수업시간에 배우지 않은 복잡한 센서/액츄에이터 코딩이 필요한 상황에서는 외부 라이브러리를 사용해 완성도를 높이도록 합시다.

### WiringPi

수업시간에 배우지 않은 내용이거나, 구현이 힘든 부분은 [WiringPi](https://github.com/WiringPi/WiringPi)라는 라이브러리를 사용할 겁니다. [WiringPi 구현 예시 코드들](https://github.com/WiringPi/WiringPi/tree/master/examples)을 참고하여 코드를 짜 봅시다.

### RPLCD

우리가 사용하는 부품 중에는 LCD가 있습니다. 이 LCD에는 I2C 모듈이 탑재되어 있어 배선이 쉽지만, C언어로 코딩하기 매우 어렵습니다. 따라서 LCD 관련 함수를 제공하는 파이썬 라이브러리인 `RPLCD`를 사용합시다.

### 컴파일 방법

```
gcc -o main main.c -l wiringPi
```

## 핀

### 할당된 핀 번호

### 핀 구조 빠르게 보는 법

`gpio readall`을 입력하면 핀 구조를 간단히 보여줍니다.
