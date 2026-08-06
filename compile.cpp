#include <iostream>
#include "src/commondata.hpp"
#include "src/cppmake.hpp"

int main(int argc, char* argv[]){
	project mainTestProj;
	//first step creating the project class
	headerFile cppmamke("./src/cppmake.hpp","cppmake.hpp");
	headerFile installmgr("./src/installmgr.hpp","installmgr.hpp");
	headerFile commonData("src/commondata.hpp","commondata.hpp");
	headerFile installmgrH("./src/installmgr.h","installmgr.h");
	//second step...creating executables obviously
	//the compiler default is set to g++ but you can always
	//mainTestProj.set_compiler("clang++");
	
	//these are global flags that are applied to all executables.
	add_flag(mainTestProj, "-Wall");
	add_flag(mainTestProj,"-Wextra");
	//the flag gets applied ONLY to main.cpp because its applied to the mainFile variable
	//adds the mainFile executable to the project ready for compilation 
	mainTestProj.addHeaderForInstall(cppmamke);
	mainTestProj.addHeaderForInstall(installmgr);
	mainTestProj.addHeaderForInstall(commonData);
	mainTestProj.addHeaderForInstall(installmgrH);
	//compilation and installation
	mainTestProj.compileAll();//despite i dont compile anything i still have to compile() to create the folders :D
	mainTestProj.preinstallAll();//prepares install yknow? ;)
	return 0;
}
