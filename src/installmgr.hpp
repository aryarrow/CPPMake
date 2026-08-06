#pragma once
#include "installmgr.h"
#include <filesystem>
#include <stdexcept>
#include <vector>
#include "commondata.hpp"
namespace installation {
	void checkInstallFolder() {
		namespace fs=std::filesystem;
		fs::path installFolder=fs::path(compilerVariables::installDir);
		if (!fs::is_directory(installFolder)){fs::create_directories(installFolder);}
		if (!fs::is_directory(installFolder / "include")){fs::create_directory(installFolder / "include");}
		if (!fs::is_directory(installFolder / "bin")){fs::create_directory(installFolder/"bin");}
		if (!fs::is_directory(installFolder / "lib")){fs::create_directory(installFolder / "lib");}
		//all good to go for installation	
	}
	
	void checkPreinstallFolder(){
		//makes sure the $(buildPath)/preinstall is made and ready for installing stuff
		namespace fs=std::filesystem;
		fs::path preinstallPath=fs::path(compilerVariables::buildDir)/"preinstall";
		fs::create_directories(preinstallPath);
		fs::create_directories(preinstallPath/"include");
		fs::create_directories(preinstallPath/"bin");
		fs::create_directories(preinstallPath/"lib");
	}

	void preinstallHeaders(const std::vector<headerFile>& headersToInstall){
		namespace fs=std::filesystem;
		
		fs::path copyToDir=fs::path(compilerVariables::buildDir)/"preinstall"/"include";//this is the $out/include
		if (!fs::is_directory(copyToDir)){
			throw std::runtime_error("Preinstall path doesnt exist, are you sure you compiled?");
		}	
		
		for (const auto& header:headersToInstall){
			fs::copy_file(fs::path(header.filename),copyToDir/header.installedName);//we'll pray this actually works without bricking stuff
		}
	}
	void preinstallBinaries(){
		namespace fs=std::filesystem;
		fs::path binDir=fs::path(compilerVariables::buildDir)/"bin";
		if (!fs::is_directory(binDir)){
			throw std::runtime_error("The output binaries dont exist, are you sure you compiled before running preinstall?");	
		}
		fs::path preinstDir=fs::path(compilerVariables::buildDir)/"preinstall"/"bin";
		
		if (!fs::is_directory(preinstDir)){
			throw std::runtime_error("Preinstalling binaries failed, are you sure you compiled?");
		}
		for (const auto& binary:fs::directory_iterator(binDir)){
			fs::copy_file(binary.path(),preinstDir/binary.path().filename());
			fs::permissions(preinstDir/binary.path().filename(),
				   fs::perms::owner_all
				   |fs::perms::group_read
				   |fs::perms::group_exec
				   |fs::perms::others_read
				   |fs::perms::others_exec
			);
		}
	}
	
	void installAll(){
		namespace fs=std::filesystem;
		checkInstallFolder();
		fs::path preinstDir=fs::path(compilerVariables::buildDir)/"preinstall";
		fs::path instDir=fs::path(compilerVariables::installDir);
		if (!fs::is_directory(preinstDir)){
			throw std::runtime_error("Installation failed, preinstall folder is missing, are you sure you compiled before hand?");
		}

		//installing binaries
		for (auto const& binary:fs::directory_iterator(preinstDir/"bin")){
			fs::copy_file(binary.path(),instDir/"bin"/binary.path().filename());//stole this from the preinstall step
			fs::permissions(instDir/"bin"/binary.path().filename(),
				   fs::perms::owner_all
				   |fs::perms::group_read
				   |fs::perms::group_exec
				   |fs::perms::others_read
				   |fs::perms::others_exec
			);
		}
		//headers
		for (auto const& header:fs::directory_iterator(preinstDir/"include")){
			fs::copy_file(header.path(),instDir/"include"/header.path().filename());
			fs::permissions(instDir/"include"/header.path().filename(),
				   fs::perms::owner_read
				   |fs::perms::owner_write
				   |fs::perms::group_read
				   |fs::perms::others_read
			);//copied this from the binaries too BUT its different because its only read/write for the owner 
			//and read only for everyone else 
		
		}
		//libs
		for (auto const& header:fs::directory_iterator(preinstDir/"lib")){
			fs::copy_file(header.path(),instDir/"lib"/header.path().filename());
			fs::permissions(instDir/"lib"/header.path().filename(),
				   fs::perms::owner_read
				   |fs::perms::owner_write
				   |fs::perms::group_read
				   |fs::perms::others_read
			);//copied this from the headers but for libs	
		}
	}
	
}

