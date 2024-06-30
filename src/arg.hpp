#pragma once

struct Args {
	int seed;
	unsigned int range;
};

enum ArgType {
	SEED_ARG,
	RANGE_ARG,
	HELP,
	LICENSE,
	ERR,
	NO_ARG,
};

Args parseArgs(int argc, char *argv[]);
