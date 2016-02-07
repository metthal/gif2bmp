#ifndef GIF_DECODER_H
#define GIF_DECODER_H

#include <cstdio>
#include <memory>
#include <stack>

#include "data_buffer.h"
#include "image.h"
#include "utils.h"

enum BlockId
{
	BLOCK_ID_EXTENSION            = 0x21,
	BLOCK_ID_IMAGE_DESCRIPTOR     = 0x2C,
	BLOCK_ID_TERMINAL             = 0x3B
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
	using ColorTable = std::vector<Color>;

	GifDecoder() = delete;
	GifDecoder(FILE *gifFile);
	~GifDecoder();

	bool decode();

	const Image* getImage() const;

protected:
	bool enoughData(std::size_t amount);
	bool nextDataBlock();

	bool decodeSignature();
	bool decodeLogicalScreenDescriptor();
	bool decodeDataBlock();
	bool decodeTableBasedImage();
	bool decodeGraphicBlock();

	const ColorTable* currentColorTable() const;
	bool newColorTable(const DataBuffer& colorTableBuffer);
	void popColorTable();

	std::unique_ptr<Image> imageFromIndexBuffer(std::uint16_t width, std::uint16_t height, const DataBuffer& indexBuffer);

private:
	FILE *_gifFile;
	std::unique_ptr<DataBuffer> _gifBuffer;
	std::size_t _decodePos;
	std::stack<ColorTable> _colorTableStack;
	std::unique_ptr<Image> _image;
};

#endif
