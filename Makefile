# 컴파일러 설정
CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -l wiringPi

# 타겟 실행 파일 이름
TARGET = program

# 소스 파일들
SRCS = main.c sensors/DHT11.c

# 목적 파일들 (.c -> .o)
OBJS = $(SRCS:.c=.o)

# 기본 타겟
all: $(TARGET)

# 실행 파일 생성 규칙
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) -l wiringPi

# 목적 파일 생성 규칙
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# clean 타겟 - 빌드 결과물 삭제
clean:
	rm -f $(OBJS) $(TARGET)

# phony 타겟 지정
.PHONY: all clean

# 의존성 설정
main.o: main.c sensors/DHT11.h sensors/DHT11.c
DHT11.o: math.c sensors/DHT11.h sensors/DHT11.c