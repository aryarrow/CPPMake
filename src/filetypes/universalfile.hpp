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
