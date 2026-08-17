#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "filetypes/universalfile.hpp"
#include "lockfilesys/locker.cpp"


//TODO:Move instances of lockfile_name in compilerVariables because it's common data between 3 f-ing classes


class lockfile_reader{
public:
	//TODO:trasform these 2 variables from below into pointers
	std::unordered_map<std::string, std::shared_ptr<File>> &files;
	std::vector<std::shared_ptr<File>> &object_dummies;
	
	/*lockfile_reader(std::unordered_map<std::string,File> &construct_files,std::vector<std::shared_ptr<File>> &construct_object_dummies):
	files(construct_files),
	object_dummies(construct_object_dummies)
	{
		locker.parse_lock_file();
		construct_files_from_lockfiles();
	}*/

	lockfile_reader(
        std::unordered_map<std::string, std::shared_ptr<File>>& construct_files,
        std::vector<std::shared_ptr<File>>& construct_object_dummies
    ) : files(construct_files), object_dummies(construct_object_dummies) {
        locker.parse_lock_file();
        construct_files_from_lockfiles();
    }

private:
	lockfile locker;
	

	void construct_files_from_lockfiles(){
		//first im getting the objects, from the lockfile
		//this is because if i actually want to use this
		for(auto& i:locker.mainVar->as<variable::object>()){
			std::string filename=i.first;
			std::string FType=i.second->get("FType")->as<std::string>();
			if (FType=="Object"){
				construct_from_lockfile(filename);
			}
		}
		//then i construct everything else
		for(auto& i:locker.mainVar->as<variable::object>()){
			std::string filename=i.first;
			std::string FType=i.second->get("FType")->as<std::string>();
			if (FType!="Object"){
				construct_from_lockfile(filename);
			}
		}
		//NOTE:Horrible performance because im going through the files twice
		//TODO:optimize this function
	}
	void construct_from_lockfile(const std::string& filename) {
        auto file_construct = std::make_shared<File>(filename, "none", File::FileType::Executable);
        file_construct->data.inputPath = filename;
        file_construct->data.outputName = locker.mainVar->get(filename)->get("outputName")->as<std::string>();

        std::string FType = locker.mainVar->get(filename)->get("FType")->as<std::string>();

        if (FType == "Executable"){ 
			file_construct->type = File::FileType::Executable;
		} else if (FType == "Object") {
			file_construct->type = File::FileType::Object;
		} else if (FType == "Header") {
			file_construct->type = File::FileType::Header;
		} else if (FType == "Sharedlib") {
			file_construct->type = File::FileType::Sharedlib;
		} else {
			throw std::logic_error("Type not implemented error:" + FType);
		}
        file_construct->generate_output_path();

		file_construct->sha256sum = locker.mainVar->get(filename)->get("sha256sum")->as<std::string>();
        
		for (const auto& i : locker.mainVar->get(filename)->get("flags")->as<variable::list>()) {
            file_construct->data.flags.push_back(i->as<std::string>());
        }

        auto search_ptr = [this](const std::string& objname) -> File* {
            for (auto& i : this->object_dummies) {
                if (i->data.inputPath == objname){return i.get();}
            }
            return nullptr;
        };

        for (auto& i : locker.mainVar->get(filename)->get("dependencies")->as<variable::list>()) {
            std::string dependency_filename = i->as<std::string>();
            file_construct->add_dependency(search_ptr(dependency_filename));
        }

        if (file_construct->type == File::FileType::Object) {
            if (!search_ptr(filename)) {
                object_dummies.push_back(file_construct);
            }
        }

        files.insert_or_assign(filename, file_construct);
    }
};

class lockfile_writer{
public:
	lockfile_writer(const std::string& construct_lockfile_name):lockfile_name(construct_lockfile_name){
		lockfile_stream=std::ofstream(lockfile_name);
		if (!lockfile_stream){
			throw std::runtime_error("Failed to create lockfile writer");
		}
		//start the file {
		lockfile_stream<<"{\n";
		depth++;
	}
	lockfile_writer& operator<<(File* file){
		//file->data ofc
		//file->dependencies this will be a list with inputNames from file->dependencies[i]->inputpath
		//file->type somehow needs to be stored but its an enum class
		//file->sha256sum
		
		//data.inputpath
		add_tabs();
		lockfile_stream<<"\""<<file->data.inputPath.string()<<"\": {\n";
		depth++;
		
		//data.outputName
		add_tabs();
		lockfile_stream<<"\"outputName\":\""<<file->data.outputName<<"\"\n";
		//data.flags
		add_tabs();
		lockfile_stream<<"\"flags\":[";
		for (const auto& i:file->data.flags){
			lockfile_stream<<"\""<<i<<"\" ";
		}
		lockfile_stream<<"]\n";
		//the FileType
		add_tabs();
		lockfile_stream<<"\"FType\":\"";
		switch (file->type) {
			case File::FileType::Executable:
				lockfile_stream<<"Executable";
			break;
			case File::FileType::Header:
				lockfile_stream<<"Header";
			break;
			case File::FileType::Object:
				lockfile_stream<<"Object";
			break;
			case File::FileType::Sharedlib:
				lockfile_stream<<"Sharedlib";
			break;
			deafult:
				throw std::logic_error("You forgot to implement a type inside the lockfile Writer");
			break;
		}
		lockfile_stream<<"\"\n";
		//dependencies(specifically their inputPath) :D
		add_tabs();
		lockfile_stream<<"\"dependencies\":[";
		for (const auto& i:file->dependencies){
			lockfile_stream<<"\""<<i->data.inputPath.string()<<"\""<<" ";
		}
		lockfile_stream<<"]\n";
		//file->sha256sum
		add_tabs();
		lockfile_stream<<"\"sha256sum\":\""<<file->sha256sum<<"\"\n";
		depth--;
		//end of the file
		add_tabs();
		lockfile_stream<<"}\n";
		return *this;
	}
	~lockfile_writer(){
		//end the file }
		lockfile_stream<<"}";
	}
private:
	int depth=0;
	std::string lockfile_name;
	std::ofstream lockfile_stream;
	void add_tabs(){
		for (int i=0;i<depth;++i) {
			lockfile_stream<<"\t";
		}
	}
};

class lockfile_mgr {
public:
    // Store shared_ptrs so pointers remain stable during map growth
    std::unordered_map<std::string, std::shared_ptr<File>> files;
    std::vector<std::shared_ptr<File>> object_dummies;

    void update_file(File& fileToUpdate) {
        auto sharedFile = std::make_shared<File>(fileToUpdate);
        files.insert_or_assign(fileToUpdate.data.inputPath.string(), sharedFile);
	}

    bool file_exists(const std::string& filename) {
        return files.find(filename) != files.end();
    }

    void update_variables_from_lockfile() {
        lockfile_reader lock_read(files, object_dummies);
    }

    void write_lockfile_changes() {
        lockfile_writer lock_write("incremental.lock");
        for (auto& i : files) {
			lock_write << i.second.get();
        }
    }
};
