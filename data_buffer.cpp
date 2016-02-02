#include <algorithm>
#include <cstring>

#include "data_buffer.h"

DataValue::DataValue() : _value()
{
}

template <typename T> DataValue::DataValue(T value)
{
	_value.resize(sizeof(T));
	memcpy(_value.data(), &value, sizeof(T));
}

template DataValue::DataValue<std::int8_t>(std::int8_t value);
template DataValue::DataValue<std::int16_t>(std::int16_t value);
template DataValue::DataValue<std::int32_t>(std::int32_t value);
template DataValue::DataValue<std::int64_t>(std::int64_t value);
template DataValue::DataValue<std::uint8_t>(std::uint8_t value);
template DataValue::DataValue<std::uint16_t>(std::uint16_t value);
template DataValue::DataValue<std::uint32_t>(std::uint32_t value);
template DataValue::DataValue<std::uint64_t>(std::uint64_t value);

DataValue::DataValue(const std::string &value) : _value()
{
	std::copy(value.begin(), value.end(), std::back_inserter(_value));
}

DataValue::DataValue(const std::vector<std::uint8_t> &value) : _value(value)
{
}

DataValue::DataValue(const DataValue &dataValue) : _value(dataValue._value)
{
}

DataValue::DataValue(DataValue &&dataValue) : _value(std::move(dataValue._value))
{
}

DataValue::~DataValue()
{
}

/**
 * Returns the amount of bytes the value is made of.
 *
 * @return Size in bytes.
 */
std::size_t DataValue::getSize() const
{
	return _value.size();
}

/**
 * Returns the boolean value of the value. The method takes sizeof(uint64_t)
 * bytes and checks whether there is at least one bit set to 1.
 *
 * @return Boolean value.
 */
bool DataValue::getBool() const
{
	return getInt<uint64_t>();
}

/**
 * Returns the value as integer type.
 *
 * @return Integer value.
 */
template <typename T> T DataValue::getInt() const
{
	T value;
	std::size_t bytesToCopy = std::min(getSize(), sizeof(T));

	memcpy(&value, _value.data(), bytesToCopy);
	return value;
}

template std::int8_t   DataValue::getInt<std::int8_t>() const;
template std::int16_t  DataValue::getInt<std::int16_t>() const;
template std::int32_t  DataValue::getInt<std::int32_t>() const;
template std::int64_t  DataValue::getInt<std::int64_t>() const;
template std::uint8_t  DataValue::getInt<std::uint8_t>() const;
template std::uint16_t DataValue::getInt<std::uint16_t>() const;
template std::uint32_t DataValue::getInt<std::uint32_t>() const;
template std::uint64_t DataValue::getInt<std::uint64_t>() const;

/**
 * Returns the value as string. The method reads bytes until null-terminator
 * is hit or the end of the buffer.
 *
 * @return String value.
 */
std::string DataValue::getString() const
{
	std::string str = "";
	std::size_t pos = 0;

	// We read until we hit null terminator or we hit the end of the buffer
	while ((_value[pos] != '\0') && (pos < getSize()))
	{
		str += _value[pos];
		pos++;
	}

	return str;
}

/**
 * Returns the value as raw bytes.
 *
 * @return Raw bytes.
 */
const std::vector<std::uint8_t>& DataValue::getBytes() const
{
	return _value;
}

DataBuffer::DataBuffer() : _data()
{
}

DataBuffer::DataBuffer(std::size_t size) : _data(size, 0)
{
}

DataBuffer::DataBuffer(const std::vector<std::uint8_t> &data) : _data(data)
{
}

DataBuffer::DataBuffer(const DataBuffer &dataBuffer) : _data(dataBuffer._data)
{
}

DataBuffer::DataBuffer(DataBuffer &&dataBuffer) : _data(std::move(dataBuffer._data))
{
}

DataBuffer::~DataBuffer()
{
}

/**
 * Creates new DataBuffer with the contents of file. The position in the file is kept as it is
 * after this method is done.
 *
 * @param file The file to read from.
 * @param offset The physical offset in the file.
 * @param amount Number of bytes to read from file.
 *
 * @return DataBuffer with contents of the file.
 */
DataBuffer DataBuffer::createFromFile(FILE *file, std::size_t offset, std::size_t amount)
{
	// Store current position in the file and move to the requested offset
	std::size_t oldPos = ftell(file);
	fseek(file, offset, SEEK_SET);

	// Read bytes from file
	std::vector<std::uint8_t> destBuffer;
	destBuffer.resize(amount);
	int bytesRead = fread(destBuffer.data(), 1, amount, file);

	// Check for error or 0 bytes read
	if (bytesRead <= 0)
	{
		fseek(file, oldPos, SEEK_SET);
		return DataBuffer();
	}

	// Make sure the buffer is the same size as read data
	destBuffer.resize(bytesRead);

	// Restore old position in the file
	fseek(file, oldPos, SEEK_SET);
	return DataBuffer(destBuffer);
}

/**
 * Returns the size of the buffer.
 *
 * @return The size of the buffer in bytes.
 */
std::size_t DataBuffer::getSize() const
{
	return _data.size();
}

/**
 * Reads the value in the buffer from the specified offset and specified size.
 *
 * @param offset The offset where to read from.
 * @param amount The number of bytes to read.
 *
 * @return DataValue representing the read value.
 */
DataValue DataBuffer::read(std::size_t offset, std::size_t amount) const
{
	// Check of boundaries
	if (offset >= getSize())
		return DataValue();

	// Calculate amount of bytes to copy in case we can run out of buffer boundaries
	std::size_t bytesToCopy = offset + amount >= getSize() ? getSize() - offset : amount;
	std::vector<std::uint8_t> bytes;
	bytes.reserve(bytesToCopy);

	// Copy using back inserter
	std::copy(_data.begin() + offset, _data.begin() + offset + bytesToCopy, std::back_inserter(bytes));
	return DataValue(bytes);
}

/**
 * Reads the specific bits from the byte at the specified offset.
 *
 * @param offset The offset of the byte.
 * @param lowBit The bit from which to start reading. 0 is LSB.
 * @param bitCount The number of bits to read from lowBit.
 *
 * @return DataValue representing the read value.
 */
DataValue DataBuffer::readBits(std::size_t offset, std::uint8_t lowBit, std::uint8_t bitCount) const
{
	// Check of boundaries
	if (offset >= getSize())
		return DataValue();

	// Read the byte at the offset and shift bits so lowBit is now new LSB
	std::uint8_t byte = _data[offset];
	byte >>= lowBit;

	// We now want bitCount bits from lowBit-th bit, so calculate bitMask to mask out right bits
	std::uint8_t bitMask = 0;
	while (bitCount)
	{
		bitMask |= (1 << bitCount);
		bitCount--;
	}

	return DataValue(byte & bitMask);
}

/**
 * Appends another DataBuffer to this DataBuffer.
 *
 * @param data DataBuffer to append.
 */
void DataBuffer::appendData(const DataBuffer& data)
{
	appendData(data._data);
}

/**
 * Appends vector of bytes to this DataBuffer.
 *
 * @param data Data to append.
 */
void DataBuffer::appendData(const std::vector<std::uint8_t> &data)
{
	std::copy(data.begin(), data.end(), std::back_inserter(_data));
}

