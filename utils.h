#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <cstdio>
#include <vector>

bool fileSize(FILE* file, std::size_t& size);
bool readFile(FILE* file, std::size_t offset, std::size_t count, std::vector<std::uint8_t>& result);

#endif
