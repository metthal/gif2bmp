#ifndef DATA_BUFFER_H
#define DATA_BUFFER_H

#include <cstdint>
#include <string>
#include <vector>

/**
 * This class stores single multi-byte value from DataBuffer. The data are stored
 * as raw bytes but they can be accessed as integral or string types using
 * interface of this class.
 */
class DataValue
{
public:
	DataValue();
	template <typename T> DataValue(T value);
	DataValue(const std::string &value);
	DataValue(const std::vector<std::uint8_t> &value);
	DataValue(const DataValue &dataValue);
	DataValue(DataValue &&dataValue);
	~DataValue();

	std::size_t getSize() const;

	bool getBool() const;
	template <typename T> T getInt() const;
	std::string getString() const;
	const std::vector<std::uint8_t>& getBytes() const;

private:
	std::vector<std::uint8_t> _value;
};

/**
 * This class represent the buffer of bytes and provides interface
 * to read the data from it.
 */
class DataBuffer
{
public:
	DataBuffer();
	DataBuffer(std::size_t size);
	DataBuffer(const std::vector<std::uint8_t> &data);
	DataBuffer(const DataBuffer &dataBuffer);
	DataBuffer(DataBuffer &&dataBuffer);
	~DataBuffer();

	static DataBuffer createFromFile(FILE *file, std::size_t offset, std::size_t amount);

	std::size_t getSize() const;

	DataValue read(std::size_t offset, std::size_t amount) const;
	DataValue readBits(std::size_t offset, std::uint8_t lowBit, std::uint8_t bitCount) const;

	void appendData(const DataBuffer &data);
	void appendData(const std::vector<std::uint8_t> &data);

private:
	std::vector<std::uint8_t> _data;
};

#endif
