#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

#include "gif_decoder.h"

#ifdef _DEBUG
static inline void print()
{
	std::cout << std::endl;
}

template <typename T, typename... Args> static inline void print(const T &val, const Args&... args)
{
	std::cout << val;
	print(args...);
}
#else
template <typename... Args> static inline void print(const Args&.../* args*/)
{
}
#endif

GifDecoder::GifDecoder(FILE *gifFile) : _gifFile(gifFile), _gifBuffer(nullptr), _decodePos(0)
{
}

GifDecoder::~GifDecoder()
{
}

bool GifDecoder::decode()
{
	_decodePos = 0;
	_gifBuffer = DataBuffer::createFromFile(_gifFile);
	if (_gifBuffer == nullptr)
		return false;

	if (!decodeSignature())
		return false;

	if (!decodeLogicalScreenDescriptor())
		return false;

	std::uint64_t blockCounter = 0;
	while (nextDataBlock())
	{
		print("Block #", blockCounter++);
		decodeDataBlock();
	}

	return true;
}

bool GifDecoder::enoughData(std::uint64_t amount)
{
	return (_gifBuffer ? (_decodePos + amount < _gifBuffer->getSize()) : false);
}

bool GifDecoder::nextDataBlock()
{
	if (!enoughData(1))
		return false;

	return (_gifBuffer->read(_decodePos, 1).getInt<std::uint8_t>() != 0x3B);
}

bool GifDecoder::decodeSignature()
{
	if (!enoughData(6))
		return false;

	DataBuffer signatureBuffer = _gifBuffer->getSubBuffer(_decodePos, 6);
	_decodePos += 6;

	std::string signature = signatureBuffer.read(0, 6).getString();
	print("GIF Version: ", signature);

	return (signature == "GIF89a" || signature == "GIF87a");
}

bool GifDecoder::decodeLogicalScreenDescriptor()
{
	if (!enoughData(7))
		return false;

	DataBuffer lsdBuffer = _gifBuffer->getSubBuffer(_decodePos, 7);
	_decodePos += 7;

	std::uint16_t gifWidth = lsdBuffer.read(0, 2).getInt<std::uint16_t>();
	std::uint16_t gifHeight = lsdBuffer.read(2, 2).getInt<std::uint16_t>();
	std::uint8_t bgColorIdx = lsdBuffer.read(5, 1).getInt<std::uint8_t>();
	bool gctPresent = lsdBuffer.readBits(4, 7, 1).getBool();

	print("Width x Height: ", gifWidth, " x ", gifHeight);
	print("Uses global color table: ", gctPresent ? "Yes" : "No");
	print("Background color index: ", static_cast<std::uint16_t>(bgColorIdx));

	if (gctPresent)
	{
		std::uint32_t gctSize = (1 << (lsdBuffer.readBits(4, 0, 3).getInt<std::uint8_t>() + 1)) * 3;
		print("Global color table size: ", gctSize, std::hex, " (0x", gctSize, ")", std::dec);

		if (!enoughData(gctSize))
			return false;

		// Global Color Table
		DataBuffer gct = _gifBuffer->getSubBuffer(_decodePos, gctSize);
		_decodePos += gctSize;
	}

	return true;
}

bool GifDecoder::decodeDataBlock()
{
	if (!enoughData(1))
		return false;

	std::uint8_t dataBlockCode = _gifBuffer->read(_decodePos++, 1).getInt<std::uint8_t>();

	// <Data> ::=                <Graphic Block>  |
	//                           <Special-Purpose Block>
	//
	// <Graphic Block> ::=       [Graphic Control Extension] <Graphic-Rendering Block>
	//
	// <Graphic-Rendering Block> ::=  <Table-Based Image>  |
	//                                Plain Text Extension
	//
	// <Table-Based Image> ::=   Image Descriptor [Local Color Table] Image Data
	//
	// <Special-Purpose Block> ::=    Application Extension  |
	//                                Comment Extension
	switch (dataBlockCode)
	{
		// <Table-Based Image>
		// Image Descriptor identifier
		case BLOCK_ID_IMAGE_DESCRIPTOR:
			print("Image Descriptor Block");
			if (!decodeTableBasedImage())
				return false;
			break;
		// Extension identifier
		case BLOCK_ID_EXTENSION:
		{
			if (!enoughData(1))
				return false;

			dataBlockCode = _gifBuffer->read(_decodePos++, 1).getInt<std::uint8_t>();
			switch (dataBlockCode)
			{
				// Plain Text Extension
				case EXTENSION_ID_PLAIN_TEXT:
					assert(false && "Plain Text Extension");
					// decodePlainTextExtension();
					break;
				// Graphic Control Extension
				case EXTENSION_ID_GRAPHIC_CONTROL:
					print("Graphic Control Extension");
					if (!decodeGraphicBlock())
						return false;
					break;
				// Commnet Extension
				case EXTENSION_ID_COMMENT:
					assert(false && "Comment Extension");
					// decodeCommentExtension();
					break;
				// Application Extension
				case EXTENSION_ID_APPLICATION:
					assert(false && "Application Extension");
					// decodeApplicationExtension();
					break;
				default:
					return false;
			}

			break;
		}
		default:
			return false;
	}

	return true;
}

bool GifDecoder::decodeTableBasedImage()
{
	if (!enoughData(9))
		return false;

	DataBuffer imgDesc = _gifBuffer->getSubBuffer(_decodePos, 9);
	_decodePos += 9;

	std::uint16_t imageX = imgDesc.read(0, 2).getInt<std::uint16_t>();
	std::uint16_t imageY = imgDesc.read(2, 2).getInt<std::uint16_t>();
	std::uint16_t imageWidth = imgDesc.read(4, 2).getInt<std::uint16_t>();
	std::uint16_t imageHeight = imgDesc.read(6, 2).getInt<std::uint16_t>();
	bool lctPresent = imgDesc.readBits(8, 7, 1).getBool();
	print("X x Y: ", imageX, " x ", imageY);
	print("Width x Height: ", imageWidth, " x ", imageHeight);
	print("Uses local color table: ", lctPresent ? "Yes" : "No");

	if (lctPresent)
	{
		std::uint32_t lctSize = (1 << (imgDesc.readBits(8, 0, 3).getInt<std::uint8_t>() + 1)) * 3;
		print("Local color table size: ", lctSize, std::hex, " (0x", lctSize, ")", std::dec);

		if (!enoughData(lctSize))
			return false;

		// Local Color Table
		DataBuffer lct = _gifBuffer->getSubBuffer(_decodePos, lctSize);
		_decodePos += lctSize;
	}

	// Image Data
	if (!enoughData(2))
		return false;

	std::uint8_t minCodeSize = _gifBuffer->read(_decodePos++, 1).getInt<std::uint8_t>();
	std::uint8_t dataSize = _gifBuffer->read(_decodePos++, 1).getInt<std::uint8_t>();

	DataBuffer compressedData;
	while (dataSize != 0)
	{
		// +1 for terminator
		if (!enoughData(dataSize))
			return false;

		DataBuffer dataSubblocks = _gifBuffer->getSubBuffer(_decodePos, dataSize);
		_decodePos += dataSize;

		compressedData.appendData(dataSubblocks);

		if (!enoughData(1))
			return false;
		dataSize = _gifBuffer->read(_decodePos++, 1).getInt<std::uint8_t>();
	}

	print("LZW compressed data with min code ", static_cast<std::uint16_t>(minCodeSize), " and size ", compressedData.getSize());
	// Run LZW decompression
	return true;
}

bool GifDecoder::decodeGraphicBlock()
{
	// Graphic Control Extension
	if (!enoughData(1))
		return false;

	std::uint8_t blockSize = _gifBuffer->read(_decodePos++, 1).getInt<std::uint8_t>();

	// +1 for terminator
	if (!enoughData(blockSize + 1))
		return false;

	_decodePos += blockSize + 1;

	if (!enoughData(1))
		return false;

	std::uint8_t dataBlockCode = _gifBuffer->read(_decodePos++, 1).getInt<std::uint8_t>();
	switch (dataBlockCode)
	{
		// Image Descriptor identifier
		case BLOCK_ID_IMAGE_DESCRIPTOR:
			print("Image Descriptor Block");
			if (!decodeTableBasedImage())
				return false;
			break;
		// Extension identifier
		case BLOCK_ID_EXTENSION:
		{
			if (!enoughData(1))
				return false;

			dataBlockCode = _gifBuffer->read(_decodePos++, 1).getInt<std::uint8_t>();
			// Plain Text Extension
			if (dataBlockCode != EXTENSION_ID_PLAIN_TEXT)
				return false;

			assert(false && "Plain Text Extension");
			break;
		}
		default:
			return false;
	}

	return true;
}
