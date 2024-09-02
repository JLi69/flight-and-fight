#include "importfile.hpp"
#include <fstream>
#include <stdio.h>

namespace impfile {
	Result::Result(bool e, std::string m)
	{
		isErr = e;
		msg = m;
	}

	Result Result::Ok()
	{
		return Result(false, "");
	}

	Result Result::Error(std::string m)
	{
		return Result(true, m);
	}

	bool Result::isError() 
	{
		return isErr;
	}

	void Result::output()
	{
		if(!isErr)
			return;
		fprintf(stderr, "E: %s\n", msg.c_str());
	}

	std::string Entry::getVar(const std::string &varname) const
	{
		if(!variables.count(varname))
			return "";
		return variables.at(varname);
	}

	std::string stripComment(std::string &line)
	{
		std::string result;
		for(auto ch : line) {
			if(ch == '#')
				return result;
			result.push_back(ch);
		}
		return result;
	}

	std::string stripWhitespace(std::string &line)
	{
		std::string result;
		//quotecount is even -> we are outside of quotes
		//quotecount is odd -> we are inside quotes
		int quotecount = 0;
		for(auto ch : line) {
			if(std::isspace(ch) && quotecount % 2 == 0)
				continue;
			if(ch == '\"')
				quotecount++;
			result.push_back(ch);
		}
		return result;
	}

	Result validateQuotes(const std::string &line)
	{
		int quotecount = 0;
		for(auto ch : line)
			if(ch == '\"')
				quotecount++;
		//If number of quotes is even, we're good
		if(quotecount % 2 == 0)
			return Result::Ok();
		else
			return Result::Error("Mismatched quotes");
	}

	Result parseName(std::string &name, std::stringstream &stream) 
	{
		char c;
		std::string result;

		int quotecount = 0;
		//Keep going until we hit '='
		while(quotecount < 2 && stream.get(c)) {
			if(c == '\"') {
				quotecount++;
				continue;
			}
			//if quotecount is 1, that means we are inside a quote
			if(quotecount == 1)
				result.push_back(c);
		}

		//If the name is empty, that means it is invalid and we must return false
		if(result.empty())
			return Result::Error("Failed to parse name!");

		name = result;
		return Result::Ok();
	}

	Result parseVariable(Variable &var, std::stringstream &stream) 
	{
		Variable v;
		char c;
		std::string* readinto = &v.first;
		//If quotecount is odd, that means we are in a quote
		//If it is even, we are outside of a quote
		int quotecount = 0;
		while(c != ';' && stream.get(c)) {
			//We hit the end, continue
			if(c == ';' && quotecount % 2 == 0)
				continue;

			if(c == '\"') {
				quotecount++;
				continue;
			}

			//Once we hit '=' switch to reading into the value
			if(c == '=' && readinto == &v.first && quotecount == 2) {
				readinto = &v.second;
				continue;
			}
			//If we are inside a string, treat an equal sign like a normal character
			else if(c == '=' && quotecount % 2 == 1) {
				(*readinto).push_back(c);
				continue;
			}
			//Syntax error
			else if(c == '=')
				return Result::Error("Extra \'=\'");

			//Syntax error
			if(quotecount % 2 == 0)
				return Result::Error("Extra characters outside of quotes");

			(*readinto).push_back(c);
		}

		//If the name of the variable is empty, that means something has gone
		//wrong and we must return false to indicate that
		if(v.first.empty())
			return Result::Error("Failed to parse variable!");

		var = v;	
		return Result::Ok();
	}

	Result parseEntry(Entry &entry, std::stringstream &stream) 
	{
		Result res = parseName(entry.name, stream);
		res.output();
		if(res.isError())
			return res;
		
		char c;
		//Assuming no whitespace, the next character should be the open
		//bracket so to ensure syntax is followed (is this technically
		//necessary? probably not though I'm enforcing it so files look
		//somewhat nice, besides it might be useful to have open and closing
		//brackets in case I want to extend this format)
		if(!stream.get(c) || c != '{')
			return Result::Error("No opening {");

		std::stringstream entryContent;
		while(stream.get(c) && c != '}')
			entryContent << c;
		
		while(entryContent.rdbuf()->in_avail()) {
			//For some weird reason the code did not work parsing variables
			//but when I just randomly put this here it magically worked for
			//some weird reason. I don't know why - it seemed fine on the
			//tests but when I tried adding it to the application it just
			//seemed to not work.
			entryContent.str(); //Anyway, don't delete this

			Variable v;
			res = parseVariable(v, entryContent);
			if(res.isError()) {
				fprintf(stderr, "Error in \'%s\'\n", entry.name.c_str());
				return res;
			}
			entry.variables.insert(v);
		}

		return Result::Ok();
	}	

	std::vector<Entry> parseFile(const char *path) 
	{
		std::vector<Entry> entries;

		std::stringstream filecontents;
		std::ifstream file(path);
		//Failed to open file, return empty vector
		if(!file.is_open()) {
			fprintf(stderr, "failed to open file: %s\n", path);
			return entries;
		}
		
		//Otherwise read the entire file's contents
		//I don't anticipate the import files to be particularly large so
		//this shouldn't cause too many issues with memory
		std::string line;
		int lineNum = 0;
		while(std::getline(file, line)) {
			lineNum++;
			line = stripComment(line);
			line = stripWhitespace(line);
			Result res = validateQuotes(line);
			if(res.isError()) {
				fprintf(stderr, "Syntax error in %s\n", path);
				res.output();
				return entries;
			}
			filecontents << line;
		}

		//Parse file contents
		while(filecontents.rdbuf()->in_avail()) {
			Entry e;
			Result res = parseEntry(e, filecontents);
			if(res.isError()) {
				fprintf(stderr, "Syntax error in %s\n", path);
				res.output();
				return entries;
			}
			entries.push_back(e);
		}

		file.close();
		return entries;
	}

	std::string entryToString(const Entry &entry) 
	{
		std::stringstream text;
		text << "\"" << entry.name << "\" {\n";
		for(const auto &var : entry.variables)
			text << "\t\"" << var.first << "\" = \"" << var.second << "\";\n";
		text << "}";
		return text.str();
	}

	void writeComment(std::ofstream &file, const char *commentText) 
	{
		std::string text;
		int index = 0;
		while(commentText[index] != '\0') {
			if(commentText[index] == '\n') {
				file << "# " << text << '\n';
				text.clear();
				index++;
				continue;
			}
			
			text.push_back(commentText[index]);
			index++;
		}

		if(!text.empty())
			file << "# " << text << '\n';
	}

	void addBoolean(Entry &entry, const std::string &name, bool b)
	{
		if(b)
			entry.variables.insert({ name, "true" });
		else
			entry.variables.insert({ name, "false" });
	}

	void addFloat(Entry &entry, const std::string &name, float f)
	{
		std::stringstream sstream;
		sstream << f;
		entry.variables.insert({ name, sstream.str() });
	}
}
