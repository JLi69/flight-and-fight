#pragma once

#include <stdio.h>
#include <assert.h>

#define TEST(testfunc) \
	printf("running %s...\n", STRINGIFY(testfunc)); \
	testfunc;

#define STRINGIFY(x) #x
#define TOSTRING STRINGIFY(x)
