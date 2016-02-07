#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <cstdio>
#include <vector>

struct Color
{
	Color() : red(0), green(0), blue(0) {}
	Color(const Color& color) : red(color.red), green(color.green), blue(color.blue) {}

	std::uint8_t red;
	std::uint8_t green;
	std::uint8_t blue;
};

bool fileSize(FILE* file, std::size_t& size);
bool readFile(FILE* file, std::size_t offset, std::size_t count, std::vector<std::uint8_t>& result);
bool writeFile(FILE* file, std::size_t offset, const std::vector<std::uint8_t>& data);

std::uint64_t alignDown(std::uint64_t value, std::uint64_t alignment);
std::uint64_t alignUp(std::uint64_t value, std::uint64_t alignment);

#endif
