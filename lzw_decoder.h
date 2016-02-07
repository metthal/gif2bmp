#ifndef LZW_DECODER_H
#define LZW_DECODER_H

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "data_buffer.h"

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

	Code* isInCodeTable(std::uint16_t code);
	void resetCodeTable(std::uint8_t& codeSize);

private:
	std::uint8_t _firstCodeSize;
	std::uint16_t _codeTableSize;
	CodeTable _codeTable;
	DataBuffer _codedData;
	Code* _lastCode;
};

#endif
