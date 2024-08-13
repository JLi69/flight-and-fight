#include "hiscore.hpp"
#include <fstream>
#include <algorithm>

void saveHighScores(const char *path, const HighScoreTable &highscores)
{
	if(highscores.empty())
		return;

	std::ofstream hiscorefile(path);

	for(auto score : highscores)
		hiscorefile << score << '\n';

	hiscorefile.close();
}

HighScoreTable loadHighScores(const char *path)
{
	HighScoreTable table = {};

	std::ifstream input(path);
	//Something went wrong, just return an empty table
	if(!input.is_open()) {
		input.close();
		return table;
	}

	std::string line;
	while(std::getline(input, line)) {
		unsigned int score = atoi(line.c_str());
		addHighScore(table, score);
	}

	//Sort the table so that everything is in order
	std::sort(table.begin(), table.end(), 
		[](unsigned int a, unsigned int b) {
			return a > b;
		});

	input.close();
	return table;
}

void addHighScore(HighScoreTable &highscores, unsigned int score)
{
	//Ignore all scores of 0
	if(score == 0)
		return;

	if(highscores.size() < HIGHSCORE_COUNT)
		highscores.push_back(score);
	else if(highscores.at(highscores.size() - 1) < score)
		highscores.at(highscores.size() - 1) = score;
	else
		return;

	std::sort(highscores.begin(), highscores.end(), 
		[](unsigned int a, unsigned int b) {
			return a > b;
		});
}
