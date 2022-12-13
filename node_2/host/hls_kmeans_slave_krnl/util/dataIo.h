
#ifndef DATA_IO_H
#define DATA_IO_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_LINE_LENGTH 2049

void random_init(float *array, const size_t N, const size_t D);
void read_file(float *array, const size_t N, const size_t D, const char *filename, bool isBinary);
void save_binary_file(float *array, const size_t N, const int D, char filename[]);
void save_text_file(float *array, const size_t N, const int D, char filename[]);

#endif
