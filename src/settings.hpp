#pragma once

/*
 * These are the settings for the game, they are imported from the file
 * 'settings.impfile' and are taken from the first entry in the file
 * (which is preferably named 'settings')
 * */

const char settingsPath[] = "settings.impfile";

class GlobalSettings {
	GlobalSettings();
public:
	//From 0.0 to 1.0
	float volume;
	bool displayCrosshair;
	void loadFromFile(const char *path);
	void save(const char *path);
	static GlobalSettings* get();
};
