#include "Prompts.h"

#include "globals.h"
#include "InputOutput.h"
//#include <nfd.h>
#include <portable-file-dialogs.h>

namespace Util
{
	/*
	std::string LoadFile(std::string ext, std::string path)
	{
		nfdchar_t * nfd_ext = nullptr;
		nfdchar_t * nfd_path = nullptr;
		nfdchar_t * outPath = nullptr;

		if (ext != "")
		{
			nfd_ext = (nfdchar_t*)ext.c_str();
		}

		if (path != "")
		{
			path = IO::relativeToAbsolutePath(path);
			nfd_path = (nfdchar_t*)path.c_str();
		}

		std::string result = "";

		nfdresult_t dialogResult = NFD_OpenDialog(nfd_ext, nfd_path, &outPath);

		if (dialogResult == NFD_OKAY)
		{
			result = std::string(outPath);
		}
		else if (dialogResult != NFD_CANCEL)
		{
			log("error loading file: {0}\n", NFD_GetError());
		}

		return result;
	}

	std::string SelectDirectory(std::string path)
	{
		nfdchar_t * nfd_path = nullptr;
		nfdchar_t * outPath = nullptr;

		if (path != "")
		{
			path = IO::relativeToAbsolutePath(path);
			nfd_path = (nfdchar_t*)path.c_str();
		}

		std::string result = "";

		nfdresult_t dialogResult = NFD_PickFolder(nfd_path, &outPath);

		if (dialogResult == NFD_OKAY)
		{
			result = std::string(outPath);
		}
		else if (dialogResult != NFD_CANCEL)
		{
			log("error selecting directory: {0}\n", NFD_GetError());
		}

		return result;
	}

	std::string SaveFile(std::string ext, std::string path, std::string defaultName)
	{
		nfdchar_t * nfd_ext = nullptr;
		nfdchar_t * nfd_path = nullptr;
		nfdchar_t * outPath = nullptr;

		if (ext != "")
		{
			nfd_ext = (nfdchar_t*)ext.c_str();
		}

		if (path != "")
		{
			path = IO::relativeToAbsolutePath(path);
			nfd_path = (nfdchar_t*)path.c_str();
		}

		if (defaultName != "")
		{
			outPath = (nfdchar_t*)defaultName.c_str();
		}

		std::string result = "";

		nfdresult_t dialogResult = NFD_SaveDialog(nfd_ext, nfd_path, &outPath);

		if (dialogResult == NFD_OKAY)
		{
			result = std::string(outPath);

			std::string dottedExt = "." + ext;
			if (result.find(dottedExt) == std::string::npos)
			{
				result = result + dottedExt;
			}
		}
		else if (dialogResult == NFD_ERROR)
		{
			const char * err = NFD_GetError();
			if (err)
			{
				log("error saving file: {0}\n", err);
			}
		}

		return result;
	}

	std::vector<std::string> LoadFiles(std::string ext, std::string path)
	{
		nfdchar_t * nfd_ext = nullptr;
		nfdchar_t * nfd_path = nullptr;
		nfdchar_t * outPath = nullptr;
		nfdpathset_t pathSet;

		if (ext != "")
		{
			nfd_ext = (nfdchar_t*)ext.c_str();
		}

		if (path != "")
		{
			path = IO::relativeToAbsolutePath(path);
			nfd_path = (nfdchar_t*)path.c_str();
		}

		std::vector<std::string> results;

		nfdresult_t dialogResult = NFD_OpenDialogMultiple(nfd_ext, nfd_path, &pathSet);

		if (dialogResult == NFD_OKAY)
		{
			for (size_t i = 0; i < NFD_PathSet_GetCount(&pathSet); ++i)
			{
				nfdchar_t * path = NFD_PathSet_GetPath(&pathSet, i);
				results.push_back(std::string(path));
			}
		}

		return results;
	}
	*/

	void init_pfd() {
		static bool initialized = false;

		if (initialized) return;

		// Check that a backend is available
		if (!pfd::settings::available())
		{
			std::cout << "Portable File Dialogs are not available on this platform.\n";
		}

		// Set verbosity to true
		#ifdef DEBUG
		pfd::settings::verbose(true);
		#endif

		initialized = true;
	}

	std::string LoadFile(const std::vector<std::string> & filters, std::string path) {
		init_pfd();

		auto f = pfd::open_file("Choose files to read", path, filters,
                            pfd::opt::multiselect);
		std::cout << "Selected files:";
		for (auto const &name : f.result())
			std::cout << " " + name;
		std::cout << "\n";

		return f.result().empty() ? "" : f.result().front();
	}

	std::string SaveFile(const std::vector<std::string>& filters, std::string path, std::string defaultName ) {
		// File save

		pfd::settings::verbose(true);
		auto f = pfd::save_file("Choose file to save", "." + pfd::path::separator() + defaultName, filters).result();
    	std::cout << "Selected file: " << f << "\n";

		return f;
	}
}

