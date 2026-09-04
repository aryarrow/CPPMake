#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include "filedata.h"


//i dont think theres anything other to do here
bool filedata::operator==(const filedata& other) const {
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

bool filedata::operator!=(const filedata& other) const {
	if (*this==other){
		return false;
	}
	return true;
}
