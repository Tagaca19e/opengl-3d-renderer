#include "InputOutput.h"
#include "StringUtil.h"
//#include "Util/Prompts.h"

#include <sstream>
#include <stdlib.h>

#if __has_include( <filesystem> )
	#include <filesystem>
#elif __has_include( <experimental/filesystem> )
	#include <experimental/filesystem>

	namespace std {
		namespace filesystem = experimental::filesystem;
	}
#else
	// Explode?
	// Force-include filesystem to at least trigger a useful error.
	#include <filesystem>
#endif

using namespace std::filesystem;

std::string IO::readText(const std::string & filename)
{
	std::string text;

	std::ifstream fin(filename, std::ios::in);

	if (fin)
	{
		std::ostringstream contents;
		contents << fin.rdbuf();
		fin.close();

		text = contents.str();
	}
	else
	{
		auto fp = relativeToAbsolutePath(filename);
		log("{0}: file not found at {1}\n", __FUNCTION__, fp);
	}

	text = StringUtil::replaceAll(text, "\r", "");

	return text;
}

bool IO::writeText(const std::string & filename, const std::string & contents, bool nullHalt)
{
	std::ofstream fout(filename);

	if (fout)
	{
		if (nullHalt)
		{
			for(auto & c : contents)
			{
				if (c == '\0')
				{
					break;
				}
				fout << c;
			}
		}
		else
		{
			fout << contents;
		}
		fout.close();
		return true;
	}
	else
	{
		//auto fp = _fullpath(nullptr, filename.c_str(), _MAX_PATH);
		log("{0}: unable to write file at {1}\n", __FUNCTION__, filename);
	}

	return false;
}

std::vector<std::string> IO::getFilenamesInFolder(const std::string & folder, const std::string & filter,
	bool recursive, bool fullPath)
{
	std::vector<std::string> results;



	std::filesystem::path p(folder);

	std::string stemFilter = "";
	std::string extFilter = "";

	// Separate the string into two parts:
	//		stem: beginning to the last .
	//		extension: last . to end
	// This lets us work with files with lots of .s: screenshot.from.last.time.jpg

	auto dotpos = filter.find_last_of('.');

	if (dotpos != std::string::npos)
	{
		stemFilter = StringUtil::clean(filter.substr(0, dotpos));
		extFilter = StringUtil::clean(filter.substr(dotpos + 1));
	}
	// If there's no . found, then just assume the filter applies entirely to the stem name
	else
	{
		stemFilter = StringUtil::clean(filter);
	}

	bool anyStem = stemFilter == "" || stemFilter == "*";
	bool anyExt = extFilter == "" || extFilter == "*";

	std::vector<directory_entry> filesInPath;

	if (recursive)
	{
		for (const auto & f : recursive_directory_iterator{ p })
		{
			if (is_regular_file(f.status()))
			{
				filesInPath.push_back(f);
			}
		}
	}
	else
	{
		for (const auto & f : directory_iterator{ p })
		{
			if (is_regular_file(f.status()))
			{
				filesInPath.push_back(f);
			}
		}
	}

	for (const auto & f : filesInPath)
	{
		bool include = true;

		const auto & _p = f.path();

		// If we have a specific stem filter and the path entry has a filename, check 'em
		if (_p.has_filename() && !anyStem)
		{
			auto filename = StringUtil::clean(_p.filename().string());

			if (filename.find(stemFilter) == std::string::npos)
			{
				continue;
			}
		}

		if (_p.has_extension() && !anyExt)
		{
			auto extension = StringUtil::clean(_p.extension().string());

			if (extension.find(extFilter) == std::string::npos)
			{
				continue;
			}
		}

		results.push_back(fullPath ? _p.string() : _p.filename().string());
	}

	// Replace back slashes with forward slashes
	for (auto & r : results)
	{
		r = StringUtil::replaceAll(r, "\\", "/");
	}

	return results;
}

std::string IO::getCurrentTimeFilename(const std::string & basename, const std::string & extension)
{
	auto now_tt = current_time_t();

	std::tm now_tm = std::tm{ 0 };
	//gmtime_s(&now_tm, &now_tt);

	return fmt::format("{0}{1}{2}-{3}-{4} {5}-{6}-{7}.{8}",
		basename, basename.empty() ? "" : " ", now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour,
		now_tm.tm_min, now_tm.tm_sec, extension);
}

std::string IO::relativeToAbsolutePath(const std::string & filename)
{
	#ifdef _WIN32
	auto fp = _fullpath(nullptr, filename.c_str(), _MAX_PATH);
	return std::string(fp);
	#else
    char result[PATH_MAX];
	if (realpath(filename.c_str(), result))
    {
        return std::string(result);
    }
    else
    {
        return "";
    }
	#endif
}

/*
IO::JsonFileIO IO::handleJsonFileIO(const json & value, const std::string & extension, const std::string & path)
{
	JsonFileIO io;

	// See if we're loading first
	if (value.find("load") != value.end())
	{
		if (value["load"].is_boolean())
		{
			io.shouldLoad = value["load"];
			io.promptFile = io.shouldLoad;
		}
		else if (value["load"].is_string())
		{
			io.filename = fmt::format("{0}/{1}", path, value["load"].get<std::string>());
			io.shouldLoad = true;
		}
	}
	else if (value.find("save") != value.end())
	{
		if (value["save"].is_boolean())
		{
			io.shouldSave = value["save"];
			io.promptFile = io.shouldSave;
		}
		else if (value["save"].is_string())
		{
			io.filename = fmt::format("{0}/{1}", path, value["save"].get<std::string>());
			io.shouldSave = true;
		}
	}

	if (io.promptFile)
	{
		if (io.shouldLoad)
		{
			//io.filename = Util::LoadFile(extension, path);
		}
		else if (io.shouldSave)
		{
			//io.filename = Util::SaveFile(extension, path);
		}

	}

	return io;
}
*/

bool IO::pathExists(const std::string & path)
{
	//auto apath = relativeToAbsolutePath(path);
	return std::filesystem::exists(path.c_str());
}

std::string IO::pathOfFile(const std::string& filename) {
	return std::filesystem::path(filename).parent_path().string();
}

std::string IO::joinPath(const std::string& path, const std::string& file) {
	return (std::filesystem::path(path) / file).string();
}

bool IO::createPath(const std::string & path)
{
	//auto apath = relativeToAbsolutePath(path);
	return std::filesystem::create_directory(path.c_str());
}

const std::string IO::getAssetRoot(const int maxDepth)
{
	static std::string assetPath;

	if (assetPath.length() == 0) {
		std::filesystem::path p = std::filesystem::absolute(".");

		for (int attempt = 0; attempt < maxDepth; attempt++) {
			if (std::filesystem::exists(p / "shaders")) { assetPath = p.string(); break; }
			if (std::filesystem::exists(p / "assets") ) { assetPath = (p /  "assets").string() ; break; }
			p = p.parent_path();
		}

		// If we didn't find the asset directory, we cannot continue.
		assert(assetPath.length() != 0);

		assetPath = std::filesystem::canonical(p).string();

		assetPath = StringUtil::replaceAll(assetPath, "\\", "/");
		//assetPath = StringUtil::replaceAll(assetPath, "/.", "/");
	}

	return assetPath;
}

std::string IO::assetPath(const std::string asset)
{
	auto test =  StringUtil::replaceAll((std::filesystem::path(IO::getAssetRoot()) / asset).string(), "\\", "/");

	return test;
}
