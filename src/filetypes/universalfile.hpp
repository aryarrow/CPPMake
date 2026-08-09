#include "filedata.hpp"
#include <filesystem>
#include <string>
#include <vector>
#include "../commondata.hpp"

class File{
public:
	enum class FileType {
		Executable,
		Object,
		Header //this is a preinstall/install feature it doesnt do anything compile time or link time
		
	};
	FileType type;
	filedata data;

	std::vector<File*> dependencies;//this is for the Executable class
	
	void add_dependency(File* file){//its just a pointer, i dont have to make it const or anything
		dependencies.push_back(file);
	}

	File(std::filesystem::path inputPath, std::string outputName,FileType fType){
		type=fType;
		data.inputPath=inputPath;
		data.outputName=outputName;
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
private:

	void generate_output_path(){
		namespace fs=std::filesystem;
		switch (type) {
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
		}
	}

};
