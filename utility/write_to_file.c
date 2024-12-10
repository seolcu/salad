#include <stdio.h>
#include <string.h>

int write_to_file(const char *filepath, const char *content)
{
    FILE *fp = fopen(filepath, "w");
    if (fp == NULL)
    {
        return -1; // 파일 열기 실패
    }

    fprintf(fp, "%s", content);
    fclose(fp);
    return 0; // 성공
}