
#include "data_buffer.h"
#include "image.h"
#include "utils.h"

Image::Image(std::uint16_t width, std::uint16_t height, const std::vector<Pixel>& pixels) : _width(width), _height(height), _pixels(pixels)
{
}

bool Image::saveBmp(FILE* outputFile) const
{
	DataBuffer outputBuffer;

	// BITMAP File Header
	outputBuffer.append(DataValue(std::string("BM"))); // Signature
	outputBuffer.append(DataValue(static_cast<std::uint32_t>(0))); // Here will come the size of file in bytes, we don't know it yet
	outputBuffer.append(DataValue(static_cast<std::uint16_t>(0))); // Reserved1
	outputBuffer.append(DataValue(static_cast<std::uint16_t>(0))); // Reserved2
	outputBuffer.append(DataValue(static_cast<std::uint32_t>(54))); // Offset to start pixel data

	// DIB Header
	outputBuffer.append(DataValue(static_cast<std::uint32_t>(40))); // Size of this header
	outputBuffer.append(DataValue(static_cast<std::uint32_t>(_width))); // Width
	outputBuffer.append(DataValue(static_cast<std::uint32_t>(_height))); // Height
	outputBuffer.append(DataValue(static_cast<std::uint16_t>(1))); // Must be 1
	outputBuffer.append(DataValue(static_cast<std::uint16_t>(24))); // Depth
	outputBuffer.append(DataValue(static_cast<std::uint32_t>(0))); // Compression method
	//outputBuffer.append(DataValue(static_cast<std::uint32_t>(_pixels.size() * 3))); // Size of pixel data
	outputBuffer.append(DataValue(static_cast<std::uint32_t>(0))); // Size of pixel data - can be zero for uncompressed BMP
	outputBuffer.append(DataValue(static_cast<std::uint32_t>(0xB13))); // Horizontal pixel per meter
	outputBuffer.append(DataValue(static_cast<std::uint32_t>(0xB13))); // Vertical pixel per meter
	outputBuffer.append(DataValue(static_cast<std::uint32_t>(0))); // Color palette size
	outputBuffer.append(DataValue(static_cast<std::uint32_t>(0))); // Can be ignored, just leave 0

	// Pixel Data
	std::size_t sizeOfData = 0;
	// BMP has data written from bottom to top and from left to right
	// So we need to start from the bottom Y line and from the leftmost X point and start writing
	for (std::int32_t y = _height - 1; y >= 0; --y)
	{
		for (std::int32_t x = 0; x < _width; ++x)
		{
			const Pixel& pixel = _pixels[(y * _width) + x];

			outputBuffer.append(DataValue(static_cast<std::uint8_t>(pixel.color.blue)));
			outputBuffer.append(DataValue(static_cast<std::uint8_t>(pixel.color.green)));
			outputBuffer.append(DataValue(static_cast<std::uint8_t>(pixel.color.red)));
			sizeOfData += 3;

			// End of scan line
			// We need to add padding
			// Scan line needs to be aligned to 4 bytes
			if (x + 1 == _width)
			{
				std::uint64_t padding = alignUp(sizeOfData, 4) - sizeOfData;
				for (std::uint64_t i = 0; i < padding; i++)
					outputBuffer.append(DataValue(static_cast<std::uint8_t>(0)));

				sizeOfData += padding;
			}
		}
	}

	outputBuffer.write(2, DataValue(static_cast<std::uint32_t>(54 + sizeOfData))); // Write back file size

	if (!outputBuffer.writeToFile(outputFile))
		return false;

	return true;
}
