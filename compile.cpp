#include <cassert>
#include <iostream>
#include "src/commondata.hpp"
#include "src/cppmake.hpp"

void print_sha(const File& file){
	std::cout<<"sha256 of file \""<<file.data.inputPath.string()<<"\" is:"<<file.sha256sum<<"\n";
}

int main(int argc, char* argv[]){
	project mainTestProj;
	File cppmake("./src/cppmake.hpp","cppmake.hpp",File::FileType::Header);
	File commondata("./src/commondata.hpp","commondata.hpp",File::FileType::Header);
	File universalfile("./src/filetypes/universalfile.hpp","filetypes/universalfile.hpp",File::FileType::Header);
	File filedata("./src/filetypes/filedata.hpp","filetypes/filedata.hpp",File::FileType::Header);
	
	File cppmaketest("./src/cppmake.hpp","cppmake.hpp",File::FileType::Header);
	
	//TODO BEFORE PRODUCTION:remove these
	print_sha(cppmake);
	print_sha(commondata);
	print_sha(universalfile);
	print_sha(filedata);
	
	
	
	mainTestProj.add_file(&cppmake);	
	mainTestProj.add_file(&commondata);	
	mainTestProj.add_file(&universalfile);	
	mainTestProj.add_file(&filedata);

	mainTestProj.preinstall();
	return 0;
}

