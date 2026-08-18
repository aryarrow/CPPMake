#pragma once
#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include "tokens.cpp"
#include "variables.cpp"
#include "tokenizer.cpp"

//TODO:create an operator<<(variable variab); which will write variab's contents into the lockfile



class lockfile{
public:
	std::unique_ptr<variable> mainVar;
	Tokenizer lockfiletokens{"incremental.lock"};
	
	void parse_lock_file(){
		//parse_brackets is next
		//checks if token_id.size() and tokens.size() are equal
		check_token_sync();
		//create the i reference
		size_t i=0;
		//parse the main brackets
		mainVar=parse_brackets(i);
		//throw an error if it didnt reach the end 
		if (!(i>=lockfiletokens.tokens.size())){
			throw std::runtime_error("We have finished parsing...but something isnt right, we parsed but we didnt reach EOF");
		}
	}

	void check_token_sync(){
		if (lockfiletokens.tokens.size()!=lockfiletokens.token_id.size()){
			throw std::runtime_error("Tokens are not syncronized with their id, what did you do wrong dear lexer?");
		}
	}

	std::unique_ptr<variable> parse_brackets(size_t& i){
		std::unique_ptr<variable> parsing_object=std::make_unique<variable>(variable(variable::object{}));

		check_i(i);
		if (lockfiletokens.token_id[i]!=Token::open_curly_bracket){
			//this can happen if the user forgets the root brackets
			throw std::runtime_error("Parser error, starting_point is not a curly bracket");
		}

		i++;
		check_i(i);

		while (i<lockfiletokens.tokens.size()){
			//first we check if its a quote for a variable or an ending
			if (!(lockfiletokens.token_id[i]==Token::quote ||
				lockfiletokens.token_id[i]==Token::closed_curly_bracket 
			)){
				throw std::runtime_error("Syntax error, unwanted token detected:"+lockfiletokens.tokens[i]);
			}
			
			//first lets make the simple string variables like "hello-world":value
			else if (lockfiletokens.token_id[i]==Token::quote){
				std::string strang=parse_string(i);
				check_equal(i);
				parse_value(i,parsing_object,strang);
			}
			//and now we check if we find the closing closing bracket
			else if (lockfiletokens.token_id[i]==Token::closed_curly_bracket){
				i++;
				return parsing_object;
			}
		}
		throw std::runtime_error("EOF reached, but bracket never closed");
		return nullptr;//this is impossible to reach due to to the throw runtime_error 
		//i added this because the compiler would give the user a warning and not compile the compiler that used the lib
	}

	lockfile()//lockfiletokens(lockfilename){
	{
		mainVar=std::make_unique<variable>(variable::object{});
	}
private:
	std::ifstream lockfile_reader;
	
	void check_equal(size_t& i){
		check_i(i);
		//check if the equal sign is = or : like "dependencies:[values]" or depencencies=[values]
		if (!(lockfiletokens.token_id[i]==Token::colon || lockfiletokens.token_id[i]==Token::equal)) {
			throw std::runtime_error("Failed to parse lock file expected : or = but we got something else");
		}
		i++;
	}

	void check_i(const size_t& i){
		if (!(i<lockfiletokens.tokens.size())){
			throw std::runtime_error("out of bounds indexing while trying to parse lock file");
		}
	}

	void parse_value(size_t& i,std::unique_ptr<variable>& parsing_object,std::string varname){
		//so we check if the value is a string, list, int or bool first 
		check_set(parsing_object);
		check_i(i);
		variable::object& obj=std::get<variable::object>(parsing_object->value);
		if (lockfiletokens.token_id[i]==Token::word){
			//check if its a word like "dependency"=word, this could only be correct in one situation, when its a bool
			bool BoolValue;
			if (lockfiletokens.tokens[i]=="true"){
				BoolValue=true;
			} else if (lockfiletokens.tokens[i]=="false") {
				BoolValue=false;
			} else {
				throw std::runtime_error("Failed to parse variable:"+varname);
			}
			obj[varname]=std::make_unique<variable>(BoolValue);
			i++;
		} else if (lockfiletokens.token_id[i]==Token::number){
			obj[varname]=std::make_unique<variable>(
				parse_number(i)
			);//so far so good
			i++;
		} else if (lockfiletokens.token_id[i]==Token::quote){
			std::string StringValue=parse_string(i);
			obj[varname]=std::make_unique<variable>(StringValue);
		} else if (lockfiletokens.token_id[i]==Token::open_square_bracket){
			//it's a list then	
			variable::list ListValue=parse_list_variable(i);
			obj[varname]=std::make_unique<variable>(std::move(ListValue));
		} else if (lockfiletokens.token_id[i]==Token::open_curly_bracket){
			obj[varname]=parse_brackets(i);//this returns the unique_ptr directly
		}
	}

	variable::list parse_list_variable(size_t& i){
		i++;//so the first element is [ we jump over that
		check_i(i);
		variable::list outputList{};
		while(i<lockfiletokens.tokens.size()){
			//i cant define these inside of switch because of a weird error
			int IntValue;
			bool BoolValue;	
			std::string StringValue;

			//checks if it's a number, boolean or string and pushes it to the list 
			//and if its a closed bracket it calls return
			
			switch (lockfiletokens.token_id[i]) {
				case Token::number:
					IntValue=parse_number(i);	
					outputList.push_back(
						std::make_unique<variable>(IntValue)
					);
				break;
				case Token::word: //this only happens with booleans like true or false
					BoolValue=parse_bool(i);
					outputList.push_back(
						std::make_unique<variable>(BoolValue)
					);
				break;
				case Token::quote:
					StringValue=parse_string(i);
					outputList.push_back(
						std::make_unique<variable>(StringValue)
					);
				break;
				case Token::closed_square_bracket:
					i++;
					return outputList;
				break;
				default:
					throw std::runtime_error("unexpected token found while parsing list:"+lockfiletokens.tokens[i]);
				break;
			}
		}

		if (lockfiletokens.token_id[i]!=Token::closed_square_bracket){
			throw std::runtime_error("Reached EOF but a list never ended");
		}
		return variable::list{};
	}

	void check_set(std::unique_ptr<variable>& set_test){
		if(set_test->get_variable_type()!="object"){
			throw std::runtime_error("Expected set got:"+set_test->get_variable_type());
		}
	}

	//Paser helpers
	//These help small things like parse_number parse_string...
	
	int parse_number(size_t& i){
		check_i(i);
		if (lockfiletokens.token_id[i]!=Token::number){
			throw std::runtime_error("A number is not a number, this could be lexer error or developer error");
		}
		int IntValue=std::stoi(lockfiletokens.tokens[i]);
		i++;
		return IntValue;
	}

	std::string parse_string(size_t& i){
		if (i+2>=lockfiletokens.tokens.size() || lockfiletokens.token_id[i+2]!=Token::quote){
			throw std::runtime_error("we detected quote for starting the string but it never ends");
		}
		if (lockfiletokens.token_id[i+1]!=Token::word){
			throw std::runtime_error("Tokenizer failed to tokenize string");
		}
		//that should be all the errors right?
		std::string strang=lockfiletokens.tokens[i+1];
		i+=3;
		return strang;
	}

	bool parse_bool(size_t& i){
		check_i(i);
		if (lockfiletokens.token_id[i]!=Token::word){
			throw std::runtime_error("Failed to parse bool, we wanted a word true or false but we got:"+lockfiletokens.tokens[i]);
		}
		if (lockfiletokens.tokens[i]=="true"){
			i++;
			return true;
		}
		if (lockfiletokens.tokens[i]=="false"){
			i++;
			return false;
		}
		throw std::runtime_error("Error:expected true or false, got:"+lockfiletokens.tokens[i]);
		return false;//this is to make the compiler warning shut up
	}
};
//Got this working with basic variables in 146 LOC
//We still have to add the object and list variables
//Update:240 LOC for a working list system....thats an extra 94 LOC
