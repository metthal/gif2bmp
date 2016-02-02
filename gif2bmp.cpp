#include "gif2bmp.h"
#include "gif_decoder.h"

int gif2bmp(tGIF2BMP * /*gif2bmp*/, FILE *inputFile, FILE *outputFile)
{
	GifDecoder gifDecoder(inputFile);
	return gifDecoder.decode() ? 0 : -1;
}
