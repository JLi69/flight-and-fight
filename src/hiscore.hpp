#pragma once

#include <vector>

//How many scores we should keep track of in the high score table
constexpr unsigned int HIGHSCORE_COUNT = 10;

//This should probably be in sorted order
typedef std::vector<unsigned int> HighScoreTable;

//Write high scores to a file
void saveHighScores(const char *path, const HighScoreTable &highscores);
//Load high scores from a file
HighScoreTable loadHighScores(const char *path);
//Add new high score, check if it exceeds the lowest high score and if it does
//then replace it and resort the table, otherwise just ignore it
//We assume the lowest score is at index 0 in the table
void addHighScore(HighScoreTable &highscores, unsigned int score);
