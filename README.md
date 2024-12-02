# 샐러드 프로젝트 가이드라인

## 라이브러리

프로젝트에서 사용할 프로그래밍 언어가 C이기 때문에, 우리는 [WiringPi](https://github.com/WiringPi/WiringPi)라는 라이브러리를 사용할 겁니다.

### 컴파일 방법

```
gcc -o main main.c -l wiringPi
```

## 핀

### 할당된 핀 번호

### 핀 구조 빠르게 보는 법

`gpio readall`을 입력하면 핀 구조를 간단히 보여줌
