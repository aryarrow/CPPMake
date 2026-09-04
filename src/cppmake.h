#include <memory>
#include <string>
#include <vector>
#include "./filetypes/universalfile.h"
#include "./lockfilemgr.h"
#include "./commondata.hpp"
//TODO:instead of all these functions, create void build(); and void preinstall();
//TODO:private the compile_executable() and compile_shared() functions
//TODO:add a compiler class for this to change with an enum class instead of an std::string
//BIG TODO:after adding the compiler class change the whole architecture to instead compile everything to objects so that they'll be linked together all at once, this makes room for a swappable linker like gnu ld or mold or anything
//BIG TODO:make the lockfile system more safe, deleting files doesnt make it error out or recompile it just keeps going...you have to reset the cache manually which sucks
class project{
public:
	project();

	void add_file(const File& fileToAppend);
	void set_compiler(const std::string& compiler);
	void compile_executables();
	void compile_shared(std::shared_ptr<File> file);
	void compile_executable(std::shared_ptr<File> file);
	
	void preinstall();
	lockfile_mgr lockfilemgr;
private:
	std::string compiler="g++";
	std::vector<std::shared_ptr<File>> files;
	
	bool should_compile(std::shared_ptr<File> objfile);
	bool run_command(const std::string& command);
	
	void compile_object(std::shared_ptr<File> objfile,bool lockfileInvolved=true);
	void copy_header_preinstall(std::shared_ptr<File> header);
	void copy_binary_preinstall(std::shared_ptr<File> binary);
	void copy_shared_preinstall(std::shared_ptr<File> shared);
	void compile_dependency_object_files(std::shared_ptr<File> executable);

	std::string quoted(const std::string& string_to_quote);
	std::string add_up_flags(std::shared_ptr<File> file);
};

void install();
