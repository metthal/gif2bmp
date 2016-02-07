#include "gif2bmp.h"
#include "gif_decoder.h"

int gif2bmp(tGIF2BMP * /*gif2bmp*/, FILE *inputFile, FILE *outputFile)
{
	GifDecoder gifDecoder(inputFile);
	if (!gifDecoder.decode())
		return -1;

	if (gifDecoder.getImage() == nullptr)
		return -1;

	if (!gifDecoder.getImage()->saveBmp(outputFile))
		return -1;

	return 0;
}
