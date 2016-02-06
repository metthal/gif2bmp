#ifndef GIF_DECODER_H
#define GIF_DECODER_H

#include <cstdio>
#include <memory>

#include "data_buffer.h"

enum BlockId
{
	BLOCK_ID_EXTENSION            = 0x21,
	BLOCK_ID_IMAGE_DESCRIPTOR     = 0x2C
};

enum ExtensionId
{
	EXTENSION_ID_PLAIN_TEXT       = 0x01,
	EXTENSION_ID_GRAPHIC_CONTROL  = 0xF9,
	EXTENSION_ID_COMMENT          = 0xFE,
	EXTENSION_ID_APPLICATION      = 0xFF
};

class GifDecoder
{
public:
	GifDecoder() = delete;
	GifDecoder(FILE *gifFile);
	~GifDecoder();

	bool decode();

protected:
	bool enoughData(std::size_t amount);
	bool nextDataBlock();

	bool decodeSignature();
	bool decodeLogicalScreenDescriptor();
	bool decodeDataBlock();
	bool decodeTableBasedImage();
	bool decodeGraphicBlock();

private:
	FILE *_gifFile;
	std::unique_ptr<DataBuffer> _gifBuffer;
	std::size_t _decodePos;
};

#endif
