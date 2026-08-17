#pragma once
#include "filedata.hpp"
#include <filesystem>
#include <string>
#include <vector>
#include "../commondata.hpp"
#include "sha256.hpp"

class File{
public:
	//TODO:add a default constructor
	//TODO:add an operator=
	enum class FileType {
		Executable,
		Object,
		Header, //this is a preinstall/install feature it doesnt do anything compile time or link time
		Sharedlib
	};
	FileType type;
	filedata data;
	std::string sha256sum;
	std::vector<File*> dependencies;//this is for the Executable class
	//TODO:transform instances of File* to shared_ptr for a better ecosystem 


	void add_dependency(File* file){//its just a pointer, i dont have to make it const or anything
		dependencies.push_back(file);
	}

	File(std::filesystem::path inputPath, std::string outputName,FileType fType){
		type=fType;
		data.inputPath=inputPath;
		data.outputName=outputName;

		SHA256::SHA256 hasher;
		sha256sum=hasher.SHA256_file(data.inputPath);
		generate_output_path();
	}

	void add_flag(const std::string& flag){
		this->data.flags.push_back(flag);
	}
	void add_library(const std::string& library){
		this->data.flags.push_back("-l"+library);
	}
	void add_library_directory(const std::filesystem::path& library){
		this->data.flags.push_back("-L"+library.string());
	}
	void add_include_directory(const std::filesystem::path& path){
		this->data.flags.push_back("-I"+path.string());
	}
	
	bool operator==(const File& other) const {
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
	bool operator!=(const File& other) const {
		if (!(*this==other)){
			return true;
		}
		return false;
	}


	void generate_output_path(){
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
};
