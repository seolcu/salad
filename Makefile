# 컴파일러 설정
CC = gcc
CFLAGS = -Wall -Wextra
# 라이브러리 링크 플래그 추가
LDFLAGS = -lwiringPi

# 타겟 실행 파일 이름
TARGET = program

# 소스 파일들. 추가하고 싶은 파일이 있으면 이곳에 추가
SRCS = main.c sensors/dht11.c sensors/soil_moisture.c

# 목적 파일들 (.c -> .o)
OBJS = $(SRCS:.c=.o)

# 기본 타겟
all: $(TARGET)

# 실행 파일 생성 규칙
$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $(TARGET)

# 목적 파일 생성 규칙
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# clean 타겟 - 빌드 결과물 삭제
clean:
	rm -f $(OBJS) $(TARGET)

# phony 타겟 지정
.PHONY: all clean