#include "arg.hpp"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <random>

//Default range
constexpr unsigned int RANGE = 8;
constexpr unsigned int MIN_RANGE = 6;
constexpr unsigned int MAX_RANGE = 48;

bool streq(const char *s1, const char *s2)
{
	if(strlen(s1) != strlen(s2))
		return false;
	return strncmp(s1, s2, strlen(s1)) == 0;
}

ArgType getArgType(const char *arg)
{
	if(streq(arg, "-s") || streq(arg, "--seed"))
		return SEED_ARG;
	if(streq(arg, "-r") || streq(arg, "--range"))
		return RANGE_ARG;
	if(streq(arg, "-h") || streq(arg, "--help"))
		return HELP;
	if(streq(arg, "--license"))
		return LICENSE;
	return ERR;
}

void usage(char *argv[])
{
	fprintf(stderr, "usage: %s -s|--seed -r|--range\n", argv[0]);
	fprintf(stderr, "-s|--seed [number]\n");
	fprintf(stderr, "\tset a seed for the world, default: random\n");
	fprintf(stderr, "-r|--range [number]\n");
	fprintf(stderr, "\tset a viewing range, default: %d chunks\n", RANGE);
	fprintf(stderr, "\tvalue should be between %d and %d\n", MIN_RANGE, MAX_RANGE);
	fprintf(stderr, "\tNOTE: setting range to a high value will result in lower performance\n");	
	fprintf(stderr, "-h|--help\n");
	fprintf(stderr, "\tshow this screen\n");
	fprintf(stderr, "--license\n");
	fprintf(stderr, "\tshow copyright info\n");
}

void license()
{
	const char COPYRIGHT[] =
		"Copyright 2024 Jeremy Li \"Nullptr Error\"\n"
		"\n"
		"Permission is hereby granted, free of charge, to any person obtaining a copy of\n"
		"this software and associated documentation files (the “Software”), to deal in the \n"
		"Software without restriction, including without limitation the rights to use, \n"
		"copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the \n"
		"Software, and to permit persons to whom the Software is furnished to do so, \n"
		"subject to the following conditions:\n"
		"\n"
		"The above copyright notice and this permission notice shall be included in all \n"
		"copies or substantial portions of the Software.\n"
		"\n"
		"THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR \n"
		"IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, \n"
		"FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE \n"
		"AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, \n"
		"WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR \n"
		"IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n";
	fprintf(stderr, "%s\n", COPYRIGHT);
}

void setArgVal(Args &argvals, ArgType argtype, int v)
{
	switch(argtype) {
	case SEED_ARG:
		argvals.seed = v;
		break;
	case RANGE_ARG:
		argvals.range = v;
	default:
		break;
	}
}

Args parseArgs(int argc, char *argv[])
{
	std::random_device rd;
	int randSeed = rd();

	Args argvals = {
		.seed = randSeed,
		.range = RANGE
	};
	ArgType arg = NO_ARG;

	for(int i = 1; i < argc; i++) {
		if(arg == NO_ARG)
			arg = getArgType(argv[i]);
		else {
			int val = atoi(argv[i]);
			setArgVal(argvals, arg, val);
			arg = NO_ARG;
		}

		switch(arg) {
		case ERR:
			fprintf(stderr, "Error parsing arguments!\n");
			usage(argv);
			exit(1);
		case HELP:
			usage(argv);
			exit(0);
		case LICENSE:
			license();
			exit(0);
		default:
			break;
		}
	}

	argvals.range = std::max(MIN_RANGE, argvals.range);
	argvals.range = std::min(MAX_RANGE, argvals.range);

	return argvals;
}
