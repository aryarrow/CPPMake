#ifdef CPPMake_impl
#include "./filetypes/universalfile.h"
#include "lockfilemgr.cpp"
#endif
#include "lockfilemgr.h"
#include "cppmake.h"
#include "./filetypes/universalfile.h"
#include "commondata.hpp"
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <csignal>
project::project(){
	std::string lockfile_name="incremental.lock";
	namespace fs=std::filesystem;
	if (fs::exists(lockfile_name)){
		lockfilemgr.update_variables_from_lockfile();
	}
}


void project::add_file(const File& fileToAppend){
	files.push_back(std::make_shared<File>(fileToAppend));
}

void project::set_compiler(const std::string& compiler){//someday ill add a Compilers class
	this->compiler=compiler;
}

void project::compile_executables(){
	for (const auto& file:files) {
		if (!file){
			throw std::runtime_error("Found nullptr while attempting to compile executables");
		}
		if (file->type==File::FileType::Executable){
			compile_executable(file);
		}
		if (file->type==File::FileType::Sharedlib){
			compile_shared(file);
		}
	}
	lockfilemgr.write_lockfile_changes();
}

void project::compile_shared(std::shared_ptr<File> file){
	if (!file) {
		throw std::logic_error("Nullptr or null was passed for compilation, you sure you wanted to do that?");
	}
	if (file->type!=File::FileType::Sharedlib){
		throw std::logic_error("A non shared lib type file was passed for shared lib compilation");
	}
	
	std::filesystem::create_directories(file->data.outputPath.parent_path());
	compile_dependency_object_files(file);

	std::string objfiles;
	for (const auto& dependency:file->dependencies){
		objfiles+=dependency->data.outputPath.string();
		objfiles+=" ";
	}

	if (!should_compile(file)){
		return;
	}
	
	std::string command=compiler+" "
		+add_up_flags(file)+" "
		+objfiles+" "+quoted(file->selfDependency->data.outputPath.string())//the objectfiles + the selfDependency
		+" -shared -o " + quoted(file->data.outputPath.string());
	//technically the command only needs to be like g++ $(object files) -shared -o outputlib
	compile_object(file->selfDependency,false);
	if(!run_command(command)){
		throw std::runtime_error("Failed to compile an executable:"+file->data.inputPath.string());
	}
	
	lockfilemgr.update_file(*file);
}

//for now we handled compilation...but what about installation??
void project::compile_executable(std::shared_ptr<File> file){
	//it's 2:25 AM its pretty difficult to think if this is correct or not
	//and after that i forgot to acutally create the directories where the f-ing executable is built
	if (!file){
		throw std::runtime_error("Nullptr or null was passed for compilation, you sure you wanted to do that?");
	}

	std::filesystem::create_directories(file->data.outputPath.parent_path());
	
	if (file->type!=File::FileType::Executable){
		throw std::runtime_error("Passed a non executable for executable compilation...");
	}
	
	compile_dependency_object_files(file);
	
	std::string objfiles;
	for (const auto& dependency:file->dependencies){//adding up dependency output file paths 
		objfiles+=dependency->data.outputPath.string();
		objfiles+=" ";
	}
	if(!should_compile(file)){
		return;
	}
	std::string command=compiler+" "
		+add_up_flags(file)+" "
		+objfiles+" "+quoted(file->selfDependency->data.outputPath.string())+" "
		+quoted(file->data.inputPath.string())+" "
		+"-o " + quoted(file->data.outputPath.string());

	compile_object(file->selfDependency,false);
	if(!run_command(command)){
		throw std::runtime_error("Failed to compile an executable:"+file->data.inputPath.string());
	}
	lockfilemgr.update_file(*file);

}

//let's do this easy, preinstall step
/*
 what does it need to do? 
 this is it for now, i'll add dynamic library support
 cp ./build/bin/ *   ./build/preinstall/
 copy the header files to their outputPath
 * */

void project::preinstall(){
	for (const auto& file:files){
		switch (file->type) {
			case(File::FileType::Executable):
				copy_binary_preinstall(file);
			break;
			case(File::FileType::Object):
			//tf is bro trying to do this is a static linked object 
				throw std::logic_error("Preinstall:Passed an object file for preinstallation");
			break;
			case(File::FileType::Header):
				copy_header_preinstall(file);
			break;
			case(File::FileType::Sharedlib):
				copy_shared_preinstall(file);
			break;
			default:
				throw std::logic_error("Preinstall:Filetype was not implemented yet");
			break;
		}
	}
  
}

void project::compile_object(std::shared_ptr<File> objfile,bool lockfileInvolved){
	//create the directory where the file will be compiled duh
	std::filesystem::create_directories(objfile->data.outputPath.parent_path());
	std::string flags=add_up_flags(objfile);
	std::string command=compiler+" " //g++ 
	+"-c "//hard coded but i dont care -c
	+flags+" " //flags without any safety regard, again i dont care :D -Wall -Wextra
	+quoted(objfile->data.inputPath.string())+" "//quoted file input name "./src/foo.hpp"
	+"-o "//hard coded again
	+quoted(objfile->data.outputPath.string());//quoted output file name "./obj/foo.o"
	if (lockfileInvolved){
		if (!should_compile(objfile)){
			return;
		}
		lockfilemgr.update_file(*objfile);
		if(!run_command(command)){
			throw std::runtime_error(
				"Failed to compile object file (incremental):"
				+objfile->data.inputPath.string()
			);
		}
	} else {
		if (!run_command(command)){
			throw std::runtime_error(
				"Failed to compile object file (nonincremental):"
				+objfile->data.inputPath.string()
			);
		}
	}
	
	
}

void project::copy_shared_preinstall(std::shared_ptr<File> shared){
	namespace fs=std::filesystem;
	if (shared->type!=File::FileType::Sharedlib){
		throw std::runtime_error("attemtped to copy non shared library to shared library:"+shared->data.inputPath.string());
	}
	//buildDir/preinstall/lib/outputName
	fs::path libdir=fs::path(compilerVariables::buildDir)/"preinstall"/"lib";
	fs::path outputShared=libdir/shared->data.outputName;
	fs::create_directories(libdir);

	fs::copy_file(shared->data.inputPath,
			   outputShared,
			   fs::copy_options::overwrite_existing
	);
}

void project::copy_header_preinstall(std::shared_ptr<File> header){
	namespace fs=std::filesystem;
	if(header->type!=File::FileType::Header){
		throw std::runtime_error("attemtped to copy non header file to header preinstall:"+header->data.inputPath.string());
	}

	//this one is a fun one beacuse outputPath is designed to be in preinstall
	fs::create_directories(header->data.outputPath.parent_path());
	fs::copy_file(header->data.inputPath,
			header->data.outputPath,
			fs::copy_options::overwrite_existing
	);
	//yes it's that simple + copy_file has by default permissions of owner_read/write and read only for the rest :D 	
}

void project::copy_binary_preinstall(std::shared_ptr<File> binary){
	namespace fs=std::filesystem;//abusing this again
	if (binary->type!=File::FileType::Executable){
		throw std::runtime_error("attempted to copy non binary file to binaries preinstall:"+binary->data.inputPath.string());
	}	
	fs::path buildDir=fs::path(compilerVariables::buildDir);
	fs::create_directories(//this syntax is weird i know basically if i have weirddir/bin/main
		buildDir/"preinstall"/"bin"/
		fs::path(binary->data.outputName).parent_path()
	);
	fs::copy_file(binary->data.outputPath,
			buildDir/"preinstall"/"bin"/binary->data.outputName,
			fs::copy_options::overwrite_existing
			);//pretty straight forward...cp ./build/bin/main ./build/preinstall/bin/
	fs::permissions(buildDir/"preinstall"/"bin"/binary->data.outputName, //copy_file revokes execute permission for some reason
				fs::perms::owner_all |
				fs::perms::group_read |
				fs::perms::group_exec |
				fs::perms::others_read |
				fs::perms::others_exec
		);	
}	

void project::compile_dependency_object_files(std::shared_ptr<File> executable){
	//executable->dependencies
	for (const auto& dependency:executable->dependencies){
		if(dependency->type==File::FileType::Object && should_compile(dependency)){
			compile_object(dependency);
		}
	}
}

std::string project::quoted(const std::string& string_to_quote){
	return "\""+string_to_quote+"\"";
}

bool project::should_compile(std::shared_ptr<File> objfile){
	//this will be implemented later for the lock file system
	std::string inputName=objfile->data.inputPath;
	//basically checks if it exists AND is equal inside the dear lockfilemgr
	if (lockfilemgr.file_exists(inputName)){
		if (*objfile==*lockfilemgr.files.at(inputName)){
			return false;
		}
	}

	return true;
}

std::string project::add_up_flags(std::shared_ptr<File> file){	
	//im acsessing shared_ptr like a regular pointer...what? you can do that???
	std::string result;
	for (const auto& flag:file->data.flags){
		result+=flag;
		result+=" ";
	}
	if(!result.empty()){
		result.pop_back();
	}
	return result;
}
bool project::run_command(const std::string& command){
	std::cout<<command<<"\n";
	int result=std::system(command.c_str());
	//In the last push i forgot to switch it from debugging to production :D
	return result==0;
	//return true;
}

void install() {//so straight forward it literally copies the preinstall directory :D
namespace fs = std::filesystem;
fs::path preinstall =
    fs::path(compilerVariables::buildDir) / "preinstall";
fs::path install =
    fs::path(compilerVariables::installDir);

fs::copy(
    preinstall,
    install,
    fs::copy_options::recursive
);
}
