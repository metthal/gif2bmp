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
	// GIF Signature
	DataBuffer gifSignature = DataBuffer::createFromFile(_gifFile, 0, 6);
	if (gifSignature.getSize() < 6)
		return false;

	// Signature has to be exactly GIF89a or GIF87a
	DataValue magicValue = gifSignature.read(0, 6);
	if (magicValue.getString() != "GIF89a" && magicValue.getString() != "GIF87a")
		return false;

	// Logical Screen Descriptor
	DataBuffer lsd = DataBuffer::createFromFile(_gifFile, 6, 7);
	if (lsd.getSize() < 7)
		return false;

	std::uint16_t imageWidth = lsd.read(0, 2).getInt<std::uint16_t>();
	std::uint16_t imageHeight = lsd.read(2, 4).getInt<std::uint16_t>();
	bool colorTablePresent = lsd.readBits(4, 7, 1).getBool();

	if (colorTablePresent)
	{
		std::uint32_t gctSize = (1 << (lsd.readBits(4, 0, 3).getInt<std::uint8_t>() + 1)) * 3;

		// Global Color Table
		DataBuffer gct = DataBuffer::createFromFile(_gifFile, 7, gctSize);
		if (gct.getSize() < gctSize)
			return false;
	}

	return true;
}
