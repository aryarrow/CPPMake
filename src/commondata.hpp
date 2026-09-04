//this one can be used for both header and literally as a c++ file cause it has a bunch of hard coded data...

#pragma once
#include <string>


namespace compilerVariables{
	std::string buildDir="./build";
	std::string installDir="./installed";
};

struct headerFile{
	std::string filename;
	std::string installedName;
	headerFile(const std::string& fName,const std::string& fInstalledName):filename(fName),installedName(fInstalledName){}
};//this does nothing during build, its only good for the
//installation phase where for example i want the user to do a /include/library.h


