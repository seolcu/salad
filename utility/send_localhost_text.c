#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// 연결을 시작하고, 메시지를 전송하고, 연결을 종료하는 함수.
int send_localhost_text(char *text, int port, int print_message)
{
    int sock = 0;
    struct sockaddr_in serv_addr;

    // 소켓 생성
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("서버와의 소켓 생성 오류\n");
        return -1;
    }

    // 서버 주소 설정
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // localhost 주소를 이진 형태로 변환
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf("주소 변환 오류\n");
        return -1;
    }

    // 서버에 연결
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("서버와 연결 실패\n");
        return -1;
    }

    // 메시지 전송
    send(sock, text, strlen(text), 0);
    if (print_message)
        printf("localhost : %d로 전송된 메시지: %s\n\n", port, text);

    close(sock);
    return 0;
}
