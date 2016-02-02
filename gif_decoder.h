#ifndef GIF_DECODER_H
#define GIF_DECODER_H

#include <cstdio>

class GifDecoder
{
public:
	GifDecoder() = delete;
	GifDecoder(FILE *gifFile);
	~GifDecoder();

	bool decode();

private:
	FILE *_gifFile;
};

#endif
