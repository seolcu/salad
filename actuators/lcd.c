#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../utility/send_localhost_text.h"

#define PORT 50000

// LCD 아랫줄에 텍스트를 출력한다.
int set_lcd_text(char *text)
{
    send_localhost_text(text, PORT, 0);
    return 0;
}