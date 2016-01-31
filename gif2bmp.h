#ifndef GIF2BMP_H
#define GIF2BMP_H

#include <cstdint>
#include <cstdio>

typedef struct
{
	int64_t bmpSize;
	int64_t gifSize;
} tGIF2BMP;

int gif2bmp(tGIF2BMP *gif2bmp, FILE *inputFile, FILE *outputFile);

#endif
