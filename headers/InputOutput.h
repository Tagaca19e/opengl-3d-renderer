#pragma once

#include "globals.h"

//#include "guid.h"

class IO
{
	public:

		static std::vector<char> readFile(const std::string& filename) {
			std::ifstream file(filename, std::ios::ate | std::ios::binary);

			if (!file.is_open()) {
				throw std::runtime_error("failed to open file!");
			}

			size_t fileSize = (size_t)file.tellg();
			std::vector<char> buffer(fileSize);

			file.seekg(0);
			file.read(buffer.data(), fileSize);
			file.close();

			return buffer;
		}

		static std::string readText(const std::string & filename);

		static bool writeText(const std::string & filename, const std::string & contents, bool nullHalt = true);


		static std::vector<std::string> getFilenamesInFolder(const std::string & folder, const std::string & filter = "*.*",
			bool recursive = false, bool fullPath = true);

		static std::string getCurrentTimeFilename(const std::string & basename, const std::string & extension);

		static std::string relativeToAbsolutePath(const std::string & filename);

		struct JsonFileIO
		{
			bool promptFile = false;
			bool shouldLoad = false;
			bool shouldSave = false;
			std::string filename = "";
		};

		/*static JsonFileIO handleJsonFileIO(const json & value, const std::string & extension = "",
			const std::string & path = "");*/

		static bool pathExists(const std::string & path);
		
		static std::string pathOfFile(const std::string& filename);

		static std::string joinPath(const std::string& path, const std::string& file);

		static bool createPath(const std::string & path);

		static const std::string getAssetRoot(const int maxDepth=5);

		static std::string assetPath(const std::string asset);
};
