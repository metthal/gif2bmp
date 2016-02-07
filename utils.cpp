#include "utils.h"

/**
 * Returns the size of the given file. This function does not modify
 * the position in the file.
 *
 * @param file The file to check.
 * @param size The size of the file.
 *
 * @return True if function was successful, otherwise false.
 */
bool fileSize(FILE* file, std::size_t& size)
{
	if (file == nullptr)
		return false;

	// Store old position in the file
	long oldPos = ftell(file);
	if (oldPos == -1L)
		return false;

	if (fseek(file, 0, SEEK_END) == -1)
		return false;

	size = ftell(file);

	// Restore old position
	fseek(file, oldPos, SEEK_SET);
	return true;
}

/**
 * Reads the contents of the file and stores it into vector buffer. Modifies the position in the file.
 *
 * @param file File to read.
 * @param offset Offset to read from.
 * @param count The number of bytes to read.
 * @param result The vector where to store result.
 *
 * @return True if read was successful, otherwise false.
 */
bool readFile(FILE* file, std::size_t offset, std::size_t count, std::vector<std::uint8_t>& result)
{
	if (file == nullptr)
		return false;

	std::size_t size;
	if (!fileSize(file, size))
		return false;

	// Check so we do not read bytes out of boundaries
	count = offset + count >= size ? size - offset : count;

	// We need to resize the buffer, so we are sure there are already enough bytes pre-allocated
	result.clear();
	result.resize(count);

	if (fseek(file, offset, SEEK_SET) == -1)
		return false;

	if (fread(result.data(), 1, count, file) != count)
		return false;

	return true;
}

bool writeFile(FILE* file, std::size_t offset, const std::vector<std::uint8_t>& data)
{
	if (file == nullptr)
		return false;

	if (fseek(file, offset, SEEK_SET) == -1)
		return false;

	if (fwrite(data.data(), 1, data.size(), file) != data.size())
		return false;

	return true;
}

std::uint64_t alignDown(std::uint64_t value, std::uint64_t alignment)
{
	return (value & ~(alignment - 1));
}

std::uint64_t alignUp(std::uint64_t value, std::uint64_t alignment)
{
	return alignDown(value + (alignment - 1), alignment);
}
