#pragma once


#include <filesystem>
#include <string>
#include <vector>
class filedata{
public:
	std::filesystem::path inputPath;
	std::string outputName;
	std::filesystem::path outputPath;
	std::vector<std::string> flags;
	
	bool operator==(const filedata& other) const;
	
	bool operator!=(const filedata& other) const;
};
