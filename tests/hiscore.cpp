#include "../src/hiscore.hpp"
#include "test.h"

void test1()
{
	HighScoreTable highscores = {
		3000,
		2005,
		2004,
		2003,
		2002,
		2001,
		2000,
		1000,
		100,
		10,
	};

	addHighScore(highscores, 1);
	assert(highscores.at(HIGHSCORE_COUNT - 1) == 10);
	addHighScore(highscores, 20);
	assert(highscores.at(HIGHSCORE_COUNT - 1) == 20);
}

void test2()
{
	HighScoreTable highscores = {
		3000,
		2005,
		2004,
		2003,
		2002,
		2001,
		2000,
		1000,
		100,
		10,
	};

	addHighScore(highscores, 101);
	assert(highscores.at(HIGHSCORE_COUNT - 1) == 100);
	assert(highscores.at(HIGHSCORE_COUNT - 2) == 101);
}

void test3()
{
	HighScoreTable highscores = {};
	for(int i = 0; i < 20; i++)
		addHighScore(highscores, 1);
	assert(highscores.size() == HIGHSCORE_COUNT);
}

int main()
{
	TEST(test1());
	TEST(test2());
	TEST(test3());
}
