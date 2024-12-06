// client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 50000
#define BUFFER_SIZE 16

// 연결을 시작하고, 메시지를 전송하고, 연결을 종료하는 함수.
int set_lcd_text(char *text)
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    char message[BUFFER_SIZE];

    // 소켓 생성
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("소켓 생성 오류\n");
        return -1;
    }

    // 서버 주소 설정
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // localhost 주소를 이진 형태로 변환
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf("주소 변환 오류\n");
        return -1;
    }

    // 서버에 연결
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("연결 실패\n");
        return -1;
    }

    printf("서버에 연결되었습니다.\n");

    // 메시지 전송
    send(sock, text, strlen(text), 0);
    printf("전송된 메시지: %s\n", text);

    close(sock);
    return 0;
}

int main()
{
    set_lcd_text("Hello, World!!!!!!!!!!");
    return 0;
}