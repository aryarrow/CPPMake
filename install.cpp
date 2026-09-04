#include <iostream>
#include <stdexcept>
#ifndef CPPMake_impl
#define CPPMake_impl
#endif

#include "./src/cppmake.cpp"
int main(int argc, char** argv){
	if (argc!=2){
		throw std::runtime_error("output install path not specified");
	}
	compilerVariables::installDir=argv[1];
	install();
	return 0;
}
