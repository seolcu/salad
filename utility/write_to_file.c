#include <stdio.h>

void write_to_file_float(char *file, char *format_string, float value)
{
    FILE *fp;
    fp = fopen(file, "w");
    fprintf(fp, format_string, value);
    fclose(fp);
}

void write_to_file_int(char *file, int value)
{
    FILE *fp;
    fp = fopen(file, "w");
    fprintf(fp, "%d", value);
    fclose(fp);
}

void write_to_file_string(char *file, char *value)
{
    FILE *fp;
    fp = fopen(file, "w");
    fprintf(fp, "%s", value);
    fclose(fp);
}