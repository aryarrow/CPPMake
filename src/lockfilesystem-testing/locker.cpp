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


inline void debug_break() {
    asm volatile("int3");
}//i looked online for how to make a breakpoint and this is what i got... im glad


class lockfile{
public:
	//BIG TODO:change all size_t to references or pointers so i dont have to literally return size_t every time
	std::unique_ptr<variable> mainVar;
	Tokenizer lockfiletokens{"flake.lock"};
	
	void parse_lock_file(){
		lockfiletokens.print_all_tokens(); //TODO BEFORE PRODUCION:Remove this, makes ouput ugly
		//parse_brackets is next
		check_token_sync();
		if (!(parse_brackets(0,mainVar)>=lockfiletokens.tokens.size())){
			throw std::runtime_error("We have finished parsing...but something isnt right, we parsed but we didnt reach EOF");
		}
	}

	void check_token_sync(){
		if (lockfiletokens.tokens.size()!=lockfiletokens.token_id.size()){
			throw std::runtime_error("Tokens are not syncronized with their id, what did you do wrong dear lexer?");
		}
	}

	size_t parse_brackets(size_t starting_point,std::unique_ptr<variable>& parsing_object){
		//TODO:make this function return the size_t index where it leaves it at
		//TODO:force commas between values
		check_i(starting_point);
		if (lockfiletokens.token_id[starting_point]!=Token::open_curly_bracket){
			//this can happen if the user forgets the root brackets
			throw std::runtime_error("Parser error, starting_point is not a curly bracket");
		}

		starting_point++;
		check_i(starting_point);
		size_t i=starting_point;

		while (i<lockfiletokens.tokens.size()){
			std::cout<<"i:"<<i<<"\n";
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
				return i;
			}
		}
		throw std::runtime_error("EOF reached, but bracket never closed");
		return 0;
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
		//TODO:add support for the rest of types found in variable class
		//Said variables that dont have support are object,list
		
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
		}
	}

	variable::list parse_list_variable(size_t& i){
		//TODO:make this parse only number,string and bool
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
		//TODO BEFORE PRODUCION:Remove these couts before production, they're ugly in the terminal
		std::cout<<"Tokens[i]:"<<lockfiletokens.tokens[i]<<"\n";
		std::cout<<"Tokens[i+1]:"<<lockfiletokens.tokens[i+1]<<"\n";
		std::cout<<"Tokens[i+2]:"<<lockfiletokens.tokens[i+2]<<"\n";
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
