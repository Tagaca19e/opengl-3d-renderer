#include "StringUtil.h"
#include "InputOutput.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace StringUtil {
	bool isEmpty(const std::string &s) {
		for (auto &c : s) {
			if (c != '\0') {
				return false;
			}
		}
		return true;
	}

	bool contains(const std::string &s, const std::string lookfor) {
		return s.find(lookfor) != std::string::npos;
	}

	std::vector<std::string> split(const std::string &s, char delim,
																bool skipEmpties) {
		std::stringstream ss(s);
		std::string item;
		std::vector<std::string> tokens;

		while (std::getline(ss, item, delim)) {
			// Add item if it's non-empty, or if skip empties is turned off
			if (!skipEmpties || !isEmpty(item)) {
				tokens.push_back(item);
			}
		}

		return tokens;
	}

	std::string replaceAll(std::string str, std::string pattern,
												std::string replacement, bool startFromBeginning) {
		std::string newStr = str;
		size_t found = newStr.find(pattern);

		while (found != std::string::npos) {
			newStr.replace(found, pattern.size(), replacement);

			if (startFromBeginning) {
				found = newStr.find(pattern);
			} else {
				found = newStr.find(pattern, found + replacement.size());
			}
		}

		return newStr;
	}

	std::string parseIncludes(const std::string &_text, const std::string &dir) {
		std::string text = _text;

		// Search for #includes
		std::string searchFor = "#include";
		size_t includeFound = text.find(searchFor);

		while (includeFound != std::string::npos) {
			// Make sure the #include isn't actually //#include
			bool validInclude = true;
			if (includeFound > 1) {
				auto isComment = text.substr(includeFound - 2, 2) == "//";
				auto charBefore = text[includeFound - 1];

				// If it's before a newline, do it!
				if (isComment || charBefore != '\n') {
					validInclude = false;
				}
			}

			if (validInclude) {
				// Find the filename in quotes immediately following an #include

				size_t firstQuote = text.find('\"', includeFound);
				size_t lastQuote = text.find('\"', firstQuote + 1);

				if (firstQuote != std::string::npos && lastQuote != std::string::npos) {
					std::string includeFilename =
							text.substr(firstQuote + 1, lastQuote - firstQuote - 1);

					std::string includeRelativePath = dir + "/" + includeFilename;

					std::string includeText =
							"\n" + IO::readText(includeRelativePath.c_str()) + "\n";

					std::string includeReplace =
							text.substr(includeFound, lastQuote - includeFound + 1);

					text = replaceAll(text, includeReplace, includeText);
				}

				// Always search from beginning in case there are nested includes >_>
				includeFound = text.find(searchFor);
			} else {
				includeFound = text.find(searchFor, includeFound + 1);
			}
		}

		return text;
	}

	std::string trim(const std::string &str) {
		auto first = str.find_first_not_of(' ');
		auto last = str.find_last_not_of(' ');
		return str.substr(first, (last - first + 1));
	}

	std::string lower(const std::string &str) {
		auto result = str;
		std::transform(result.begin(), result.end(), result.begin(), ::tolower);
		return result;
	}

	std::string upper(const std::string &str) {
		auto result = str;
		std::transform(result.begin(), result.end(), result.begin(), ::toupper);
		return result;
	}

	std::string clean(const std::string& str) {
		return lower(trim(str));
	}

	std::string readText(const char *textFile) {
		std::FILE *fp = std::fopen(textFile, "rb");
		if (fp) {
			std::string contents;
			std::fseek(fp, 0, SEEK_END);
			contents.resize(std::ftell(fp));
			std::rewind(fp);
			std::fread(&contents[0], 1, contents.size(), fp);
			std::fclose(fp);
			return contents;
		} else {
			std::perror("Error opening file");
			throw std::runtime_error("Failed to open file");
		}
	}
} // namespace StringUtil
