#include <cstdint>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <memory>

#include "gif2bmp.h"

enum ArgsFlags
{
	ARGS_NONE         = 0,
	ARGS_INPUT_FILE   = 1,
	ARGS_OUTPUT_FILE  = 2,
	ARGS_LOG_FILE     = 4,
	ARGS_HELP         = 8
};

struct ArgsInfo
{
	ArgsInfo() : flags(ARGS_NONE), inputFileName(""), outputFileName(""), logFileName("") {}

	uint32_t flags;
	std::string inputFileName;
	std::string outputFileName;
	std::string logFileName;
};

void printHelp()
{
	std::cout
		<< "GIF2BMP\n"
		<< "GIF to BMP Converter\n"
		<< "Author: Marek Milkovic in 2016 for KKO course at FIT BUT\n"
		<< "\n"
		<< "Usage:\n"
		<< "    gif2bmp [options]\n"
		<< "\n"
		<< "Options:\n"
		<< "    -h                          Prints this help message.\n"
		<< "    -i <ifile>                  Specifies input GIF file. If not specified, STDIN is used.\n"
		<< "    -o <ofile>                  Specifies output BMP file. If not specified, STDOUT is used.\n"
		<< "    -l <logfile>                Specified file for logging messages. If not specified, no logging messages are generated."
		<< std::endl;
}

bool parseArgs(ArgsInfo& argsInfo, int argc, char *argv[])
{
	int opt;
	while ((opt = getopt(argc, argv, "i:o:l:h")) != -1)
	{
		switch (opt)
		{
			case 'i':
				argsInfo.flags |= ARGS_INPUT_FILE;
				argsInfo.inputFileName = optarg;
				break;
			case 'o':
				argsInfo.flags |= ARGS_OUTPUT_FILE;
				argsInfo.outputFileName = optarg;
				break;
			case 'l':
				argsInfo.flags |= ARGS_LOG_FILE;
				argsInfo.logFileName = optarg;
				break;
			case 'h':
				argsInfo.flags |= ARGS_HELP;
				break;
			default:
				return false;
		}
	}

	return true;
}

bool processArgs(const ArgsInfo &argsInfo, tGIF2BMP& convReport)
{
	// Print help if -h was specified
	if (argsInfo.flags & ARGS_HELP)
	{
		printHelp();
		return true;
	}

	// Handle input file
	FILE* input = stdin;
	if (argsInfo.flags & ARGS_INPUT_FILE)
	{
		FILE *fi = fopen(argsInfo.inputFileName.c_str(), "r");
		if (fi == nullptr)
			return false;

		input = fi;
	}

	// Handle output file
	FILE* output = stdout;
	if (argsInfo.flags & ARGS_OUTPUT_FILE)
	{
		FILE *fo = fopen(argsInfo.outputFileName.c_str(), "w");
		if (fo == nullptr)
			return false;

		output = fo;
	}

	// Handle log file
	FILE* log = nullptr;
	if (argsInfo.flags & ARGS_LOG_FILE)
	{
		FILE *fl = fopen(argsInfo.logFileName.c_str(), "w");
		if (fl == nullptr)
			return false;

		log = fl;
	}

	int result = gif2bmp(&convReport, input, output);

	// Cleanup
	if (output != stdout)
		fclose(output);
	if (input != stdin)
		fclose(input);
	if (log != nullptr)
		fclose(log);

	return (result == 0);
}

int main(int argc, char *argv[])
{
	ArgsInfo argsInfo;
	if (!parseArgs(argsInfo, argc, argv))
	{
		printHelp();
		return 1;
	}

	tGIF2BMP convReport;
	return processArgs(argsInfo, convReport) ? 0 : 1;
}
