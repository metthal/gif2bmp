#ifndef LZW_DECODER_H
#define LZW_DECODER_H

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "data_buffer.h"

const std::uint16_t MAX_CODE_SIZE = 12;

class LzwDecoder
{
public:
	struct Code
	{
		Code() : reset(false), end(false), index(0), data() {}
		Code(const Code& code) : reset(code.reset), end(code.end), index(code.index), data(code.data) {}

		bool reset;
		bool end;
		std::uint16_t index;
		std::vector<std::uint8_t> data;
	};

	using CodeTable = std::unordered_map<std::uint16_t, Code>;

	LzwDecoder(std::uint8_t firstCodeSize, std::uint16_t codeTableSize, const DataBuffer& codedData);

	bool decode(DataBuffer& decodedData);

protected:
	bool isResetCode(std::uint16_t code);
	bool isEndCode(std::uint16_t code);

	bool getNextCode(std::uint16_t& code);
	void createNewCode(std::uint16_t index, std::uint8_t appendByte);

	Code* isInCodeTable(std::uint16_t code);
	bool resetCodeTable(DataBuffer& decodedData);

private:
	std::uint8_t _firstCodeSize;
	std::uint8_t _codeSize;
	std::uint64_t _readPos;
	std::uint16_t _initCodeTableSize;
	CodeTable _codeTable;
	DataBuffer _codedData;
	Code* _lastCode;
};

#endif
