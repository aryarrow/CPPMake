#pragma once
#include <filesystem>
#include <string>
#include <vector>

class filedata {
public:
	std::filesystem::path inputPath;
	std::string outputName;
	std::filesystem::path outputPath;
	std::vector<std::string> flags;
	bool operator==(const filedata& other) const {
		if(other.inputPath!=this->inputPath){
			return false;
		}
		if(other.outputName!=this->outputName){
			return false;
		}
		if(other.outputPath!=this->outputPath){
			return false;
		}
		//does c++ 17 have std::vector<> operator==??? 
		if(other.flags!=this->flags){
			return false;
		}
		return true;
	}

	bool operator!=(const filedata& other) const {
		if (*this==other){
			return false;
		}
		return true;
	}
};
