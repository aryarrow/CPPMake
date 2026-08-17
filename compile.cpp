#include <cassert>
#include <iostream>
#include "src/commondata.hpp"
#include "src/cppmake.hpp"


int main(int argc, char* argv[]){
	//since this entire project is a library i couldve literally used a recursive_directory_iterator to header all of them :D
	project CPPMake;
	//for filetype
	File filedata("./src/filetypes/filedata.hpp","filetypes/filedata.hpp",File::FileType::Header);
	File universalfile("./src/filetypes/universalfile.hpp","filetypes/universalfile.hpp",File::FileType::Header);
	File sha256("./src/filetypes/sha256.hpp","filetypes/sha256.hpp",File::FileType::Header);

	//root
	File cppmake("./src/cppmake.hpp","cppmake.hpp",File::FileType::Header);
	File commondata("./src/commondata.hpp","commondata.hpp",File::FileType::Header);
	File lockmgr("./src/lockfilemgr.cpp","lockfilemgr.cpp",File::FileType::Header);
	File GDBbreakpoint("./src/breakpoint.cpp","breakpoint.cpp",File::FileType::Header);
	//lockfilesys
	File locktokens("./src/lockfilesys/tokens.cpp","lockfilesys/tokens.cpp",File::FileType::Header);
	File locktokenizer("./src/lockfilesys/tokenizer.cpp","lockfilesys/tokenizer.cpp",File::FileType::Header);
	File lockparser("./src/lockfilesys/locker.cpp","lockfilesys/locker.cpp",File::FileType::Header);
	File lockvariables("./src/lockfilesys/variables.cpp","lockfilesys/variables.cpp",File::FileType::Header);

	auto pushFile=[&CPPMake](File &file_to_push){
		CPPMake.add_file(&file_to_push);
	};
	pushFile(filedata);
	pushFile(universalfile);
	pushFile(sha256);

	pushFile(cppmake);
	pushFile(commondata);
	pushFile(lockmgr);
	pushFile(GDBbreakpoint);

	pushFile(locktokens);
	pushFile(locktokenizer);
	pushFile(lockparser);
	pushFile(lockvariables);

	CPPMake.preinstall();
	return 0;
}

