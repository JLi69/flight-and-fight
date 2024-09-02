#include "../src/importfile.hpp"
#include "test.h"

void test1()
{
	std::vector<impfile::Entry> entries = 
		impfile::parseFile("impfiles/test1.impfile");

	assert(entries.size() == 1);
	impfile::Entry e = entries.at(0);
	assert(e.name == "entry1");
	assert(e.variables.size() == 2);
	assert(e.variables.count("var1"));
	assert(e.variables.count("var2"));
	assert(e.variables.at("var1") == "hello");
	assert(e.variables.at("var2") == "good bye");
}

void test2()
{
	std::vector<impfile::Entry> entries = 
		impfile::parseFile("impfiles/test2.impfile");

	assert(entries.size() == 3);
	
	std::vector<impfile::Entry> expected = {
		{
			.name = "foo",
			.variables = {
				{ "a", "bar" },
				{ "b", "baz" },
				{ "c", "buzz" },
			}
		},

		{
			.name = "foo2",
			.variables = {
				{ "var1", "fizz" },
				{ "var2", "buzz" },
			}
		},

		{
			.name = "foo3",
			.variables = {}
		}
	};

	assert(entries.size() == expected.size());
	for(int i = 0; i < expected.size(); i++) {
		assert(entries.at(i).name == expected.at(i).name);
		for(auto v : expected.at(i).variables) {
			assert(entries.at(i).variables.count(v.first));
			assert(
				entries.at(i).variables.at(v.first) ==
				v.second
			);
		}
	}
}

void test3() 
{
	std::vector<impfile::Entry> entries = 
		impfile::parseFile("impfiles/test3.impfile");

	assert(entries.empty());
}

void test4()
{
	std::vector<impfile::Entry> entries = 
		impfile::parseFile("impfiles/test4.impfile");

	assert(entries.empty());
}

void test5()
{
	impfile::Entry entry;
	entry.name = "test";
	entry.variables.insert({ "foo", "bar" });
	entry.variables.insert({ "fizz", "buzz" });
	std::string str = impfile::entryToString(entry);
	str = impfile::stripWhitespace(str);
	std::stringstream strstream(str);
	
	impfile::Entry entry2;
	impfile::Result res = impfile::parseEntry(entry2, strstream);
	res.output();
	assert(!res.isError());
	assert(entry2.name == entry.name);
	for(const auto &var : entry.variables) {
		assert(entry2.variables.count(var.first));
		assert(entry2.variables.at(var.first) == var.second);
	}
}

int main()
{
	TEST(test1());
	TEST(test2());
	TEST(test3());
	TEST(test4());
	TEST(test5());
}
