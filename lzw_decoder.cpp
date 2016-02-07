#include <cassert>
#include <algorithm>
#include <iostream>

#include "lzw_decoder.h"

LzwDecoder::LzwDecoder(std::uint8_t firstCodeSize, std::uint16_t codeTableSize, const DataBuffer& codedData) :
	_firstCodeSize(firstCodeSize), _codeTableSize(codeTableSize), _codeTable(), _codedData(codedData), _lastCode(nullptr)
{
}

bool LzwDecoder::decode(DataBuffer& decodedData)
{
	std::uint64_t readPos = 0;
	std::uint8_t codeSize = _firstCodeSize;

	// Not enough bits in coded data
	if (readPos + codeSize >= (_codedData.getSize() * 8))
		return false;
	std::uint16_t firstCode = _codedData.readBits(readPos, codeSize).getInt<std::uint16_t>();
	readPos += codeSize;

	// First code needs to be always reset
	if (!isResetCode(firstCode))
		return false;
	resetCodeTable(codeSize);

	// Not enough bits in coded data
	if (readPos + codeSize >= (_codedData.getSize() * 8))
		return false;
	firstCode = _codedData.readBits(readPos, codeSize).getInt<std::uint16_t>();
	readPos += codeSize;

	// Code after reset is just read, its value should be in code table, index is written to output and it is remembered as last code
	Code* firstCodeObj = isInCodeTable(firstCode);
	if (firstCodeObj == nullptr)
		return false;
	decodedData.append(firstCodeObj->data);
	_lastCode = firstCodeObj;

	while (true)
	{
		// Not enough bits in coded data
		if (readPos + codeSize >= (_codedData.getSize() * 8))
			return false;

		Code* codeObj = nullptr;
		std::uint16_t code = _codedData.readBits(readPos, codeSize).getInt<std::uint16_t>();
		readPos += codeSize;

		std::cout << "Current code: " << code
			<< " last code: " << (_lastCode ? _lastCode->index : -1)
			<< std::endl;
		if (isEndCode(code))
		{
			break;
		}
		else if (isResetCode(code))
		{
			resetCodeTable(codeSize);

			// Not enough bits in coded data
			if (readPos + codeSize >= (_codedData.getSize() * 8))
				return false;
			code = _codedData.readBits(readPos, codeSize).getInt<std::uint16_t>();
			readPos += codeSize;

			// Code after reset is just read, its value should be in code table, index is written to output and it is remembered as last code
			codeObj = isInCodeTable(code);
			if (codeObj == nullptr)
				return false;
			decodedData.append(codeObj->data);
			_lastCode = codeObj;
			continue;
		}
		else if ((codeObj = isInCodeTable(code)) != nullptr)
		{
			assert(_lastCode);
			decodedData.append(codeObj->data);

			Code newCode;
			newCode.index = _codeTable.size();
			newCode.data = _lastCode->data;
			newCode.data.push_back(codeObj->data[0]);
			_codeTable.insert( { newCode.index, newCode } );
			std::cout << "Added #" << newCode.index << " ";
			std::for_each(newCode.data.begin(), newCode.data.end(), [](std::uint8_t x) { std::cout << static_cast<std::uint16_t>(x) << ","; });
			std::cout << std::endl;

			if (newCode.index == ((1 << codeSize) - 1))
				codeSize++;
		}
		else
		{
			assert(_lastCode);
			decodedData.append(_lastCode->data);
			decodedData.append(_lastCode->data[0]);

			Code newCode;
			newCode.index = _codeTable.size();
			newCode.data = _lastCode->data;
			newCode.data.push_back(_lastCode->data[0]);
			_codeTable.insert( { newCode.index, newCode } );
			std::cout << "Added #" << newCode.index << " ";
			std::for_each(newCode.data.begin(), newCode.data.end(), [](std::uint8_t x) { std::cout << static_cast<std::uint16_t>(x) << ","; });
			std::cout << std::endl;

			if (newCode.index == ((1 << codeSize) - 1))
				codeSize++;
		}

		_lastCode = isInCodeTable(code);
	}

	return true;
}

bool LzwDecoder::isResetCode(std::uint16_t code)
{
	if (_codeTable.empty())
		return code == _codeTableSize;

	Code* codeObj = isInCodeTable(code);
	return codeObj ? codeObj->reset : false;
}

bool LzwDecoder::isEndCode(std::uint16_t code)
{
	if (_codeTable.empty())
		return code == _codeTableSize + 1;

	Code* codeObj = isInCodeTable(code);
	return codeObj ? codeObj->end : false;
}

void LzwDecoder::resetCodeTable(uint8_t& codeSize)
{
	codeSize = _firstCodeSize;
	//_lastCode = nullptr;

	_codeTable.clear();
	for (std::uint16_t i = 0; i < _codeTableSize; ++i)
	{
		Code newCode;
		newCode.reset = newCode.end = false;
		newCode.index = i;
		newCode.data = { static_cast<std::uint8_t>(i) };
		_codeTable.insert( { newCode.index, newCode } );
	}

	Code resetCode;
	resetCode.reset = true;
	resetCode.end = false;
	resetCode.index = _codeTableSize;
	resetCode.data = {};
	_codeTable.insert( { resetCode.index, resetCode } );

	Code endCode;
	endCode.reset = false;
	endCode.end = true;
	endCode.index = _codeTableSize + 1;
	endCode.data = {};
	_codeTable.insert( { endCode.index, endCode } );
}

LzwDecoder::Code* LzwDecoder::isInCodeTable(std::uint16_t code)
{
	auto itr = _codeTable.find(code);
	if (itr == _codeTable.end())
		return nullptr;

	return &(itr->second);
}
