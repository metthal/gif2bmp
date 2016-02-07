#include <cassert>
#include <algorithm>
#include <iostream>

#include "lzw_decoder.h"

LzwDecoder::LzwDecoder(std::uint8_t firstCodeSize, std::uint16_t codeTableSize, const DataBuffer& codedData) :
	_firstCodeSize(firstCodeSize), _codeSize(firstCodeSize), _readPos(0), _initCodeTableSize(codeTableSize), _codeTable(), _codedData(codedData), _lastCode(nullptr)
{
}

bool LzwDecoder::decode(DataBuffer& decodedData)
{
	std::uint16_t code;
	if (!getNextCode(code))
		return false;

	// First code needs to be always reset
	if (!isResetCode(code))
		return false;

	if (!resetCodeTable(decodedData))
		return false;

	while (true)
	{
		if (!getNextCode(code))
			return false;

		// Find the code in code table
		Code* codeObj = isInCodeTable(code);

		if (isEndCode(code))
		{
			break;
		}
		else if (isResetCode(code))
		{
			if (!resetCodeTable(decodedData))
				return false;

			continue;
		}
		// Existing code
		else if (codeObj != nullptr)
		{
			assert(_lastCode);
			decodedData.append(codeObj->data);

			// For existing code, create new code with the first free index, what is size of code table
			createNewCode(_codeTable.size(), codeObj->data[0]);
		}
		// New code
		else
		{
			assert(_lastCode);
			decodedData.append(_lastCode->data);
			decodedData.append(_lastCode->data[0]);

			// We need to create new code with this non-existing code index
			createNewCode(code, _lastCode->data[0]);
		}

		_lastCode = isInCodeTable(code);
	}

	return true;
}

bool LzwDecoder::isResetCode(std::uint16_t code)
{
	if (_codeTable.empty())
		return code == _initCodeTableSize;

	Code* codeObj = isInCodeTable(code);
	return codeObj ? codeObj->reset : false;
}

bool LzwDecoder::isEndCode(std::uint16_t code)
{
	if (_codeTable.empty())
		return code == _initCodeTableSize + 1;

	Code* codeObj = isInCodeTable(code);
	return codeObj ? codeObj->end : false;
}

bool LzwDecoder::getNextCode(std::uint16_t& code)
{
	// Not enough bits in coded data
	if (_readPos + _codeSize > (_codedData.getSize() * 8))
		return false;

	code = _codedData.readBits(_readPos, _codeSize).getInt<std::uint16_t>();
	_readPos += _codeSize;
	return true;
}

bool LzwDecoder::resetCodeTable(DataBuffer& decodedData)
{
	_codeSize = _firstCodeSize;

	_codeTable.clear();
	for (std::uint16_t i = 0; i < _initCodeTableSize; ++i)
	{
		Code newCode;
		newCode.reset = newCode.end = false;
		newCode.index = i;
		newCode.data = { static_cast<std::uint8_t>(i) };
		_codeTable.insert( { newCode.index, newCode } );
	}

	// Clear code
	Code resetCode;
	resetCode.reset = true;
	resetCode.end = false;
	resetCode.index = _initCodeTableSize;
	resetCode.data = {};
	_codeTable.insert( { resetCode.index, resetCode } );

	// End of Information code
	Code endCode;
	endCode.reset = false;
	endCode.end = true;
	endCode.index = _initCodeTableSize + 1;
	endCode.data = {};
	_codeTable.insert( { endCode.index, endCode } );

	// Not enough bits in coded data
	std::uint16_t code;
	if (!getNextCode(code))
		return false;

	// Code after reset is just read, its value should be in code table, index is written to output and it is remembered as last code
	Code* codeObj = isInCodeTable(code);
	if (codeObj == nullptr)
		return false;

	decodedData.append(codeObj->data);
	_lastCode = codeObj;
	return true;
}

void LzwDecoder::createNewCode(std::uint16_t index, std::uint8_t appendByte)
{
	Code newCode;
	newCode.index = index;
	newCode.data = _lastCode->data;
	newCode.data.push_back(appendByte);
	_codeTable.insert( { newCode.index, newCode } );

	// If the new index is maximum value on _codeSize number of bytes and _codeSize is less than 12
	//   then we need to increase the _codeSize
	if (newCode.index >= ((1 << _codeSize) - 1))
		if (_codeSize < MAX_CODE_SIZE)
			_codeSize++;
}

LzwDecoder::Code* LzwDecoder::isInCodeTable(std::uint16_t code)
{
	auto itr = _codeTable.find(code);
	if (itr == _codeTable.end())
		return nullptr;

	return &(itr->second);
}
