#pragma once

#include <string>
#include <vector>

namespace Util
{
	std::string LoadFile(const std::vector<std::string> & filters= {}, std::string path = "");
	//std::string SelectDirectory(std::string path = "");
	std::string SaveFile(const std::vector<std::string>& filters = {}, std::string path = "", std::string defaultName = "");
	//std::vector<std::string> LoadFiles(std::string ext = "", std::string path = "");
	
}