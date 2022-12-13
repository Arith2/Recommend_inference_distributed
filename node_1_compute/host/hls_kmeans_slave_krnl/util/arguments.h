
#ifndef MKM_ARGS_H
#define MKM_ARGS_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
void parsing_args(int argc, char* argv[], size_t *n, size_t *k, size_t *d, int *threads, char *filename, bool isBinary, uint32_t*iter, uint32_t *repetition, int* batch_size, float* threshold);

#endif //KMEANS_ARGS_H
