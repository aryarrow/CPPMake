#pragma once
#include "universalfile.h"
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include "../commondata.hpp"
#include "sha256.hpp"

//Thankfully i only use sha256.hpp in the implementation and i dont have to create a .h file for it too :D

void File::generate_self_dependency(){
	std::shared_ptr<File> selfObject;
	switch (this->type) {
		case File::FileType::Sharedlib:
			selfObject=std::make_shared<File>(
				this->data.inputPath,this->data.outputName+".o",File::FileType::Object
			);

			selfObject->add_flag("-fPIC");	
			selfDependency=selfObject;
		break;
		case File::FileType::Executable:
			selfObject=std::make_shared<File>(
				this->data.inputPath,this->data.outputName+".o",File::FileType::Object
			);
			selfDependency=selfObject;
		break;
	}
		


}

void File::add_dependency(const File& file){//its just a pointer, i dont have to make it const or anything
	File fileCopy=file;
	if(fileCopy.type!=File::FileType::Object) {
		throw std::logic_error("Attempted to append a non object as a dependency");
	}
	if(this->type==File::FileType::Sharedlib){
		fileCopy.add_flag("-fPIC");
	}
	dependencies.push_back(std::make_shared<File>(fileCopy));
}

File::File(std::filesystem::path inputPath, std::string outputName,FileType fType){
	type=fType;
	data.inputPath=inputPath;
	data.outputName=outputName;

	SHA256::SHA256 hasher;
	sha256sum=hasher.SHA256_file(data.inputPath);
	generate_output_path();
	generate_self_dependency();	
}

//TODO:once adding the compiler class change these based on the compiler
void File::add_flag(const std::string& flag){
	this->data.flags.push_back(flag);
	if (!this->selfDependency.get()){
		return;
	}
	this->selfDependency->add_flag(flag);
}
void File::add_library(const std::string& library){
	this->data.flags.push_back("-l"+library);
	if (!this->selfDependency.get()){
		return;
	}
	this->selfDependency->add_flag("-l"+library);
}
void File::add_library_directory(const std::filesystem::path& library){
	this->data.flags.push_back("-L"+library.string());
	if (!this->selfDependency.get()){
		return;
	}
	this->selfDependency->add_flag("-L"+library.string());
}
void File::add_include_directory(const std::filesystem::path& path){
	this->data.flags.push_back("-I"+path.string());
	if (!this->selfDependency.get()){
		return;
	}
	this->selfDependency->add_flag("-I"+path.string());
}

bool File::operator==(const File& other) const {
	if (other.type!=this->type){
		return false;
	}
	if (this->data!=other.data){
		return false;
	}
	if (this->dependencies.size()!=other.dependencies.size()){
		return false;//this is not actually for comparison its for the for loop to not seg fault :D
	}
	//compare the hahses
	if (this->sha256sum!=other.sha256sum) {
		return false;
	}
	//compare the dependencies
	for (std::size_t i=0;i<this->dependencies.size();++i){
		if (*this->dependencies[i]!=*other.dependencies[i]){
			return false;
		}
	}
	return true;
}
bool File::operator!=(const File& other) const {
	if (!(*this==other)){
		return true;
	}
	return false;
}


void File::generate_output_path(){
	namespace fs=std::filesystem;
	switch (this->type) {
		case FileType::Executable:
			data.outputPath=fs::path(compilerVariables::buildDir)/"bin"/data.outputName;
		break;
		case FileType::Object:
			data.outputPath=fs::path(compilerVariables::buildDir)/"obj"/data.outputName;
		break;
		case FileType::Header:
			data.outputPath=fs::path(compilerVariables::buildDir)/ //./build
				"preinstall"/ //preintall
				"include"/ //include
				data.outputName; // mylib/something.h
		break;
		case FileType::Sharedlib:
			data.outputPath=fs::path(compilerVariables::buildDir)/
				"lib"/
				data.outputName;
		break;
	}
}
