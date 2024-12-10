#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <locale.h>
#include <wchar.h>
#include "../utility/send_localhost_text.h"

#define PORT 50001
#define MAX_JSON_SIZE 4096

// UTF-8 문자열을 JSON용으로 이스케이프하는 함수
void escape_json_string(const char *input, char *output, size_t output_size)
{
    unsigned int i = 0, j = 0;

    // 출력 버퍼의 시작에 큰따옴표 추가
    output[j++] = '"';

    while (input[i] != '\0' && j < output_size - 3)
    { // 닫는 따옴표와 널 문자를 위한 공간 확보
        unsigned char c = input[i];

        if (c == '"')
        {
            // 큰따옴표 이스케이프
            output[j++] = '\\';
            output[j++] = '"';
        }
        else if (c == '\\')
        {
            // 백슬래시 이스케이프
            output[j++] = '\\';
            output[j++] = '\\';
        }
        else if (c < 32)
        {
            // 제어 문자 이스케이프
            switch (c)
            {
            case '\b':
                output[j++] = '\\';
                output[j++] = 'b';
                break;
            case '\f':
                output[j++] = '\\';
                output[j++] = 'f';
                break;
            case '\n':
                output[j++] = '\\';
                output[j++] = 'n';
                break;
            case '\r':
                output[j++] = '\\';
                output[j++] = 'r';
                break;
            case '\t':
                output[j++] = '\\';
                output[j++] = 't';
                break;
            default:
                // 다른 제어 문자는 \u00XX 형식으로 이스케이프
                snprintf(output + j, output_size - j, "\\u%04x", c);
                j += 6;
                break;
            }
        }
        else if (c < 128)
        {
            // ASCII 문자는 그대로 복사
            output[j++] = c;
        }
        else
        {
            // UTF-8 바이트는 그대로 복사
            output[j++] = c;
        }
        i++;
    }

    // 출력 버퍼의 끝에 큰따옴표 추가
    output[j++] = '"';
    output[j] = '\0';
}

int tts_talk(char *text)
{
    // JSON 메시지 생성을 위한 버퍼
    char escaped_text[MAX_JSON_SIZE];
    char json_message[MAX_JSON_SIZE];

    // 텍스트를 JSON 용으로 이스케이프
    escape_json_string(text, escaped_text, sizeof(escaped_text));

    // 버퍼 오버플로우 체크를 추가한 버전
    int len = snprintf(json_message, sizeof(json_message),
                       "{\"text\": %s}", escaped_text);
    if (len < 0 || (unsigned int)len >= sizeof(json_message))
    {
        printf("Error: Message too long\n");
        return -1;
    }

    send_localhost_text(json_message, PORT, 1);
    return 0;
}
