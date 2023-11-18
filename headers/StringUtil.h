#pragma once

#include <string>
#include <vector>

namespace StringUtil
{
	bool isEmpty(const std::string & s);

	bool contains(const std::string& s, const std::string lookfor);

	std::vector<std::string> split(const std::string & s, char delim, bool skipEmpties = true);

	// Returns a copy of "str" with all instances of "pattern" replaced by "replacement"
	std::string replaceAll(std::string str, std::string pattern, std::string replacement, bool startFromBeginning = true);

	std::string parseIncludes(const std::string & text, const std::string & dir = "");

	std::string trim(const std::string & str);

	std::string lower(const std::string & str);

	std::string upper(const std::string & str);

	std::string clean(const std::string& str);

	std::string readText(const char* textFile);
}
