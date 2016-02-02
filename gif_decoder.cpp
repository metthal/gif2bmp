#include <cstdint>
#include <vector>

#include "gif_decoder.h"
#include "data_buffer.h"

GifDecoder::GifDecoder(FILE *gifFile) : _gifFile(gifFile)
{
}

GifDecoder::~GifDecoder()
{
}

bool GifDecoder::decode()
{
	DataBuffer gifSignature = DataBuffer::createFromFile(_gifFile, 0, 6);
	if (gifSignature.getSize() < 6)
		return false;

	DataValue magicValue = gifSignature.read(0, 6);
	if (magicValue.getString() != "GIF89a")
		return false;

	return true;
}
