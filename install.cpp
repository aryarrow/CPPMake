#include <iostream>
#include <stdexcept>
#include "./src/cppmake.hpp"
int main(int argc, char** argv){
	if (argc!=2){
		throw std::runtime_error("output install path not specified");
	}
	compilerVariables::installDir=argv[1];
	install();
	return 0;
}
