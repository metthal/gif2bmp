#include <algorithm>
#include <cstring>

#include "data_buffer.h"
#include "utils.h"

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

DataValue& DataValue::operator =(DataValue &&dataValue)
{
	_value = std::move(dataValue._value);
	return *this;
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

DataBuffer::DataBuffer(const DataBuffer &dataBuffer, std::size_t offset, std::size_t count)
{
	count = offset + count >= dataBuffer.getSize() ? dataBuffer.getSize() - offset : count;
	_data.reserve(count);
	std::copy(dataBuffer._data.begin() + offset, dataBuffer._data.begin() + offset + count, std::back_inserter(_data));
}

DataBuffer::DataBuffer(DataBuffer &&dataBuffer) : _data(std::move(dataBuffer._data))
{
}

DataBuffer::~DataBuffer()
{
}

DataBuffer& DataBuffer::operator =(DataBuffer &&dataBuffer)
{
	_data = std::move(dataBuffer._data);
	return *this;
}

/**
 * Creates new DataBuffer with the contents of the whole file.
 *
 * @param file The file to read from.
 *
 * @return DataBuffer with contents of the file.
 */
std::unique_ptr<DataBuffer> DataBuffer::createFromFile(FILE *file)
{
	if (file == nullptr)
		return nullptr;

	std::size_t size;
	if (!fileSize(file, size))
		return nullptr;

	std::vector<std::uint8_t> contents;
	if (!readFile(file, 0, size, contents))
		return nullptr;

	return std::make_unique<DataBuffer>(contents);
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
 * Creates the copy of the sub-buffer from the given offset up to given number of bytes.
 *
 * @param offset The offset to copy from.
 * @param amount The number of bytes to copy.
 *
 * @return Sub-buffer.
 */
DataBuffer DataBuffer::getSubBuffer(std::size_t offset, std::size_t amount) const
{
	return DataBuffer(*this, offset, amount);
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
 * Reads the specific bits from the byte at the specified offset. Method also
 * access neighbour elements if bitCount overlaps current byte. No more than
 * 64 bits are read.
 *
 * @param byteOffset The offset of the byte.
 * @param bitOffset The bit from which to start reading. 0 is LSB.
 * @param bitCount The number of bits to read.
 *
 * @return DataValue representing the read value.
 */
DataValue DataBuffer::readBits(std::size_t byteOffset, std::uint8_t bitOffset, std::size_t bitCount) const
{
	if (byteOffset >= getSize())
		return DataValue();

	if (bitOffset >= 8)
		return DataValue();

	// We don't read more than 64 bits
	if (bitCount >= (sizeof(std::uint64_t) * 8))
		bitCount = 64;

	std::uint8_t currentByte = _data[byteOffset] >> bitOffset;
	std::size_t bitsWritten = 0;
	std::uint64_t val = 0;
	while (bitCount > 0)
	{
		// We take the current byte and mask it out so only required number of bits is taken into account
		// We then shift it right so it fits right position based on the amount of bits already read
		val |= static_cast<std::uint64_t>((currentByte & _getBitMask(bitCount))) << bitsWritten;

		// We now need to calculate how many bits we have written
		// If there is less than 8 bits left, use that count as number of bits we have written
		// Otherwise, calculate it with formula 8 - bitOffset
		if (bitCount < 8)
			bitsWritten += bitCount;
		else
			bitsWritten += 8 - bitOffset;
		bitCount -= bitsWritten;

		byteOffset++;
		if (byteOffset >= getSize())
			break;

		currentByte = _data[byteOffset];
	}

	return DataValue(val);
}

/**
 * Reads the specific bits from the bit at the specified offset. Method also
 * access neighbour elements if bitCount overlaps current byte. No more than
 * 64 bits are read.
 *
 * @param bitOffset The bit from which to start reading. 0 is LSB of whole buffer.
 * @param bitCount The number of bits to read.
 *
 * @return DataValue representing the read value.
 */
DataValue DataBuffer::readBits(std::size_t bitOffset, std::size_t bitCount) const
{
	// Dividing by 8 to get index of the byte
	std::size_t byteOffset = bitOffset >> 3;

	// Modulo 8 to get bit in byte offset
	std::uint8_t bitInByteOffset = bitOffset & 7;

	return readBits(byteOffset, bitInByteOffset, bitCount);
}

std::uint8_t DataBuffer::_getBitMask(std::size_t bitCount)
{
	// We don't use breaks in switch intentionally
	// We first set mask to highest possible (0xFF) and then abuse fall through switch
	// and set bits 0.
	// The order of cases is very important!!! Do not change!!!
	std::uint8_t mask = 0xFF;
	switch (bitCount)
	{
		case 0:
			mask &= ~0x01;
		case 1:
			mask &= ~0x02;
		case 2:
			mask &= ~0x04;
		case 3:
			mask &= ~0x08;
		case 4:
			mask &= ~0x10;
		case 5:
			mask &= ~0x20;
		case 6:
			mask &= ~0x40;
		case 7:
			mask &= ~0x80;
		default:
			break;
	}

	return mask;
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

