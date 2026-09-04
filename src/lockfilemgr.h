#pragma once
#include "./filetypes/universalfile.h"
#include "./lockfilesys/locker.h"
#include <fstream>
#include <memory>
#include <unordered_map>
class lockfile_reader{
public:
	std::unordered_map<std::string, std::shared_ptr<File>> &files;
	std::vector<std::shared_ptr<File>> &object_dummies;

	lockfile_reader (
		std::unordered_map<std::string, std::shared_ptr<File>>& construct_files,
		std::vector<std::shared_ptr<File>>& construct_object_dummies
	) : files(construct_files),object_dummies(construct_object_dummies) {
		locker.parse_lock_file();
		construct_files_from_lockfile();
	}

	private:
	lockfile locker;
	void construct_files_from_lockfile();
	void construct_from_lockfile(const std::string& filename);
};




class lockfile_writer{
public:
	lockfile_writer(const std::string& construct_lockfile_name);
	lockfile_writer& operator<<(File* file);
	~lockfile_writer();
private:
	int depth=0;
	std::string lockfile_name;
	std::ofstream lockfile_stream;
	void add_tabs();
};




class lockfile_mgr{
public:
	//storing them here in a higher class than the lockfile_reader
	std::unordered_map<std::string,std::shared_ptr<File>> files;
	std::vector<std::shared_ptr<File>> object_dummies;

	void update_file(File& fileToUpdate);
	bool file_exists(const std::string& filename);//this checks in the lockfile its not std::filesystem::exists ok?
	void update_variables_from_lockfile();
	void write_lockfile_changes();
};
