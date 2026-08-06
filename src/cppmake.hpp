#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include "commondata.hpp"
#include "installmgr.hpp"
#include "installmgr.h"


class sourceFile{
public:
	std::string filename;
	std::string outputName;
	sourceFile(const std::string& fName,const std::string& fOutputName):filename(fName),outputName(fOutputName){}


	std::vector<std::string> flags;
};

class executableFile{
public:
	std::string filename;
	std::string fileOutputName;
	executableFile(const std::string& inputName,const std::string& outputName):filename(inputName),fileOutputName(outputName){}


	std::vector<std::string> flags;//ill let the user be able to just flags.push_back
	std::vector<sourceFile> sources;
};

class project{
public:
	void add_executable(const executableFile& thefile){
		executables.push_back(thefile);
	}

	void set_compiler(const std::string& compiler){
		this->compiler=compiler;
	}

	void preinstallAll(){
		installation::checkPreinstallFolder();
		if (!compiledFiles){//check if the files were compiled
			throw std::runtime_error("Compilation was not done before installation");
		}
		installation::preinstallBinaries();
		installation::preinstallHeaders(headersToInstall);
		//for now theres no shared lib support due to my F-ING OPERATING system
		//i really hope i dont have to send this code to the jury, cause im cooked by how many emoji coments i have
	}

	void compileAll(){
		checkBuildDirs();
		if (!compileDependencyObjectFiles(this->sources)){
			throw std::runtime_error("Global dependency file compilation failed");
		}//compile the global object files
		for (const auto &executable:executables){
			if (!compile_executable_file(executable)){
				throw std::runtime_error("Something went wrong during compilation of "+executable.filename);
			}
		}
		compiledFiles=true;
	}


	bool compile_executable_file(const executableFile& execution){
		if (!compileDependencyObjectFiles(execution.sources)){
			return 0;
		}
		std::string universalFlags=addFlags(flags);
		std::string command=compiler+" "+//clang++
			execution.filename+" "//main.cpp for example
			+getDependencyObjectOutputNames(this->sources)+" "//adds global object files
			+getDependencyObjectOutputNames(execution.sources)+" "//adds local or private object files
			+universalFlags//-Wall -Wextra for example
			+addFlags(execution.flags)//addFlags accidentally adds another space at the end so its "-flag -luke "(with the space)
			+"-o "+compilerVariables::buildDir+"/bin/"+execution.fileOutputName;//./build/bin/a.out? idk man
		//this will basically form the command g++ main.cpp -Wall -Wextra(these are universal flags) -DhelloWorld(perFileFlag) -o main
		//TODO:add object file support
		return runcommand(command);
	}

	void addHeaderForInstall(const headerFile& headerToInstall){
		headersToInstall.push_back(headerToInstall);
	}//i dont need to explain what the function does, its simple enough to be understood

	std::vector<std::string> flags;//flags that are applied to ALL files
	std::vector<executableFile> executables;//.cpp or .c files that will be compiled to binary files like main.cpp->main
	std::vector<sourceFile> sources;//source files that will get compiled to .o files example math.cpp->math.o
	std::vector<headerFile> headersToInstall;//we dont talk about this one during the build phase ok? its install only stuff



private:
	bool compiledFiles=false;//this variable is for the install phase
	std::string compiler="g++";

	bool compileDependencyObjectFiles(const std::vector<sourceFile>& objfiles){
		//recursive object file compilation
		//now we hope the user didn't make the dependency second.o with third.o and the other way around to not create a paradox
		for (const auto& source:objfiles){
			std::string command;//let's attempt to form the command to compile
			command=this->compiler+" "//clang++
				+source.filename+" "
				+addFlags(this->flags)+" "//universal flags like -Wall -Wextra or smh
				+addFlags(source.flags)+" "//per obeject file flags like -fsanitize=address or idk -g maybe?
				+"-c "//this time make sure they get turned into a damn object file
				+"-o "+compilerVariables::buildDir+"/obj/"+source.outputName;//resulting source
			if (!runcommand(command)){
				return false;
			}
		}
		return 1;
	}

	std::string getDependencyObjectOutputNames(const std::vector<sourceFile>& dependencies){
		std::string result;
		for (const auto& dependency:dependencies){
			result+=compilerVariables::buildDir+"/obj/";
			result+=dependency.outputName;
			result+=" ";
		}
		return result;
	}

	std::string addFlags(const std::vector<std::string>& flags){
		std::string addedUp=" ";
		for (const auto& flag : flags){
			addedUp+=flag;
			addedUp+=" ";
		}
		return addedUp;
	}

	bool runcommand(const std::string& command){
		std::cout<<command<<"\n";
		return (system(command.c_str())==0);
	}
	void checkBuildDirs(){
		namespace fs=std::filesystem;
		fs::create_directories(fs::path(compilerVariables::buildDir)/"obj");
		fs::create_directories(fs::path(compilerVariables::buildDir)/"bin");
	}//makes sure the buildDir directories exist ready for use	
};
template <class T> void add_flag(T& Object,const std::string& flag){
	Object.flags.push_back(flag);
}

template <class T> void add_library(T& Object,const std::string& libName){
	Object.flags.push_back("-l"+libName);//i'll not do dependency handling because this project will integrate nix into it
}

template <class T>
void add_library_directory(T& object, const std::string& path) {
	object.flags.push_back("-L" + path);
}

template <class T>
void add_include_directory(T& object, const std::string& path) {
	object.flags.push_back("-I" + path);
}

template <class T>
void add_source_file(T& object,const sourceFile& srcfile){
	object.sources.push_back(srcfile);
}
