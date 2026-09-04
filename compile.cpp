#include <cassert>
#include <iostream>
#include <filesystem>
#include <vector>
#ifndef CPPMake_impl
#define CPPMake_impl
#endif
#include "./src/cppmake.cpp"



int main(int argc, char* argv[]){
	project CPPMake;
	std::vector<File> headers;
	//lets steal the headers
	namespace fs=std::filesystem;
	for (const auto& file:fs::recursive_directory_iterator{"./src"}){
		fs::path relativePath=file.path().lexically_relative("./src");
		if (relativePath.extension().string()==".h"){
			File temporary(file.path().string(),relativePath.string(),File::FileType::Header);
			headers.push_back(temporary);
		}
	}
	for (auto const& i:headers){
		CPPMake.add_file(i);
	}
	//okay now create the shared library
	File sharedlib("./src/cppmake.cpp","libcppmake.so",File::FileType::Sharedlib);
	CPPMake.add_file(sharedlib);

	CPPMake.compile_executables();
	CPPMake.preinstall();
	return 0;
}

