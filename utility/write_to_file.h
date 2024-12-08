#ifndef WRITE_TO_FILE_H
#define WRITE_TO_FILE_H

void write_to_file_float(char *file, char *format_string, float value);
void write_to_file_int(char *file, int value);
void write_to_file_string(char *file, char *value);

#endif