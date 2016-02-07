#ifndef IMAGE_H
#define IMAGE_H

#include <cstdint>
#include <vector>

#include "utils.h"

class Image
{
public:
	struct Point
	{
		Point() : x(0), y(0) {}
		Point(const Point& point) : x(point.x), y(point.y) {}

		std::uint16_t x;
		std::uint16_t y;
	};

	struct Pixel
	{
		Pixel() : coord(), color() {}
		Pixel(const Pixel& pixel) : coord(pixel.coord), color(pixel.color) {}

		Point coord;
		Color color;
	};

	Image(std::uint16_t width, std::uint16_t height, const std::vector<Pixel>& pixels);

	bool saveBmp(FILE* outputFile) const;

private:
	std::uint16_t _width;
	std::uint16_t _height;
	std::vector<Pixel> _pixels;
};

#endif
