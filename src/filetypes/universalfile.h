#pragma once
#include "./filedata.h"

#ifdef CPPMake_impl
#include "./filedata.hpp"
#endif

#include <filesystem>
#include <memory>
#include <string>
#include <vector>


class File{
public:
	enum class FileType{
		Executable,
		Object,
		Header,//preinstall/install feature only it doesnt compile anything
		Sharedlib
	};
	FileType type;
	filedata data;
	std::string sha256sum;
	std::vector<std::shared_ptr<File>> dependencies;
	std::shared_ptr<File> selfDependency;

	void add_dependency(const File& file);

	File(std::filesystem::path inputPath,std::string outputName,FileType fType);


	//TODO:once creating the compiler class please change these so that they use the compiler class instead
	void add_flag(const std::string& flag);
	void add_library(const std::string& library);
	void add_library_directory(const std::filesystem::path& library);
	void add_include_directory(const std::filesystem::path& path);
	bool operator==(const File& other) const;
	bool operator!=(const File& other) const;

	void generate_output_path();
	private:
	void generate_self_dependency();
};
