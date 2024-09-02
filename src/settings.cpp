#include "settings.hpp"
#include "importfile.hpp"
#include <fstream>

const char settingsFileComment[] =
	"This is the file that stores the settings for Flight & Fight.\n"
	"You can modify it manually though it is recommended that you\n"
	"instead use the in-game GUI.\n"
	"Anyway, if you are reading this, I hope you have a nice day :)\n";

GlobalSettings::GlobalSettings()
{
	//Inititalize default settings
	values.volume = 1.0f;
	values.canDisplayCrosshair = true;
}

void GlobalSettings::loadFromFile(const char *path)
{
	std::vector<impfile::Entry> entries = impfile::parseFile(path);

	if(entries.empty())
		return;

	impfile::Entry settings = entries.at(0);
	
	if(settings.getVar("display_crosshair") == "false")
		values.canDisplayCrosshair = false;
	else
		values.canDisplayCrosshair = true;

	std::string volumeStr = settings.getVar("volume");
	if(volumeStr.empty())
		values.volume = 1.0f;
	else
		values.volume = atof(volumeStr.c_str());
}

void GlobalSettings::save(const char *path)
{
	std::ofstream settingsfile(path);

	if(!settingsfile.is_open()) {
		settingsfile.close();
		fprintf(stderr, "Failed to open: %s\n", path);
		return;
	}

	impfile::writeComment(settingsfile, settingsFileComment);

	impfile::Entry entry;
	entry.name = "settings";
	impfile::addBoolean(entry, "display_cursor", values.canDisplayCrosshair);
	impfile::addFloat(entry, "volume", values.volume);
	std::string filecontents = impfile::entryToString(entry);

	settingsfile << filecontents << '\n';
	settingsfile.close();
}

GlobalSettings* GlobalSettings::get()
{
	static GlobalSettings* settings = new GlobalSettings;
	return settings;
}
