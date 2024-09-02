#pragma once
#include <string>
#include <unordered_map>
#include <sstream>
#include <vector>

/*
 * an import file (impfile) is of the following format:
 *
 * # This is a comment
 * # You can not nest entries
 * "name" {
 *		# This is another comment
 *		# anything in quotes must be on one line
 *		"var1" = "value"
 *		"var2" = "value"
 *		"var3" = "value"
 *		...
 * }
 * */

namespace impfile {
	typedef std::pair<std::string, std::string> Variable;

	struct Entry {
		std::string name;
		//variable name -> value
		std::unordered_map<std::string, std::string> variables;
		std::string getVar(const std::string &varname) const;
	};

	class Result {
		bool isErr;
		std::string msg;
		Result(bool e, std::string m);
	public:
		bool isError();
		void output();
		static Result Ok();
		static Result Error(std::string m);
	};

	//returns true if every quote is closed, false if otherwise
	Result validateQuotes(const std::string &line);
	//Takes in a line and removes all characters after a '#' character
	//and returns the resulting string
	//assumes line has no newline characters
	std::string stripComment(std::string &line);
	//Removes all the whitespace from a string and returns the resulting string
	std::string stripWhitespace(std::string &line);
	
	//These parse functions assume all whitespace has been stripped out
	//and all comments have been stripped out as well
	//returns true if it can successfully parse the name
	Result parseName(std::string &name, std::stringstream &stream);
	//returns true if it can successfully parse the variable
	Result parseVariable(Variable &var, std::stringstream &stream);
	//returns true if it can successfully parse an entry
	Result parseEntry(Entry &entry, std::stringstream &stream);
	
	std::vector<Entry> parseFile(const char *path);
	
	//converts an entry into a string
	std::string entryToString(const Entry &entry);
	//Writes a comment to an impfile
	void writeComment(std::ofstream &file, const char *commentText);
	void addBoolean(Entry &entry, const std::string &name, bool b);
	void addFloat(Entry &entry, const std::string &name, float f);
}
