#ifndef DATA_BUFFER_H
#define DATA_BUFFER_H

#include <cstdint>
#include <memory>
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

	DataValue& operator =(DataValue &&dataValue);

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
	DataBuffer(const DataBuffer &dataBuffer, std::size_t offset, std::size_t count);
	DataBuffer(DataBuffer &&dataBuffer);
	~DataBuffer();

	DataBuffer& operator =(DataBuffer &&dataBuffer);

	static std::unique_ptr<DataBuffer> createFromFile(FILE *file);

	std::size_t getSize() const;
	DataBuffer getSubBuffer(std::size_t offset, std::size_t amount) const;

	DataValue read(std::size_t offset, std::size_t amount) const;
	DataValue readBits(std::size_t byteOffset, std::uint8_t bitOffset, std::size_t bitCount) const;
	DataValue readBits(std::size_t bitOffset, std::size_t bitCount) const;

	void appendData(const DataBuffer &data);
	void appendData(const std::vector<std::uint8_t> &data);

private:
	static std::uint8_t _getBitMask(std::size_t bitCount);

	std::vector<std::uint8_t> _data;
};

#endif
