#pragma once
#include <fstream>
#include <memory>
#include <string>
#include "variables.h"
#include "tokenizer.h"
class lockfile{
public:
	std::shared_ptr<variable> mainVar;
	Tokenizer lockfiletokens{"incremental.lock"};
	
	void parse_lock_file();
	std::shared_ptr<variable> parse_brackets(size_t& i);
	void check_token_sync();
	
	lockfile();
private:
	std::ifstream lockfile_reader;

	void check_equal(size_t& i);
	void check_i(const size_t& i);
	void check_set(std::shared_ptr<variable>& set_test);
	void parse_value(size_t& i,std::shared_ptr<variable> parsing_object,std::string varname);//This is wrong...
	//TODO:change parse_value so that it returns a shared_ptr and doesnt f-ing change an existing one
	
	
	variable::list parse_list_variable(size_t& i);
	int parse_number(size_t& i);
	std::string parse_string(size_t& i);
	bool parse_bool(size_t& i);
};
