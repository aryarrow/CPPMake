#pragma once
#ifdef CPPMake_impl
#include "tokens.cpp"
#endif

#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>
#include <string>
#include "tokens.h"
#include <iostream>
#include "tokenizer.h"

void Tokenizer::print_all_tokens(){
	for (size_t i=0;i<tokens.size();++i){
			std::cout<<tokens[i]<<":"<<token_id[i]<<"\n";
	}
}
Tokenizer::Tokenizer(const std::string& filename){
	namespace fs=std::filesystem;
	fs::path file=fs::path(filename);//only doing ts because of windows
	//who cant take an std::string instead of std::path like linux can :D
	if (!fs::exists(file)){
		throw std::runtime_error("File not found:"+filename);
	}
	if (!fs::is_regular_file(file)){
		throw std::runtime_error("File is not a regular file:"+filename);
	}
	std::ifstream fin(filename);//almost wrote "filename" instead of filename 
	//because of muscle memory
	if (!fin){
		throw std::runtime_error("Failed to open file:"+filename);
	}
	std::string text;
	std::stringstream text_buffer;
	text_buffer<<fin.rdbuf();
	text=text_buffer.str();
	tokenize(text);
}


	void Tokenizer::tokenize(const std::string& text){
		for (std::size_t i=0;i<text.size();++i){
			char ch=text[i];
			std::string remember;
			if(isalpha(static_cast<unsigned char>(ch))){
				remember+=ch;
				i++;
				while (i<text.size() && isalnum(static_cast<unsigned char>(text[i]))){
					remember+=text[i];
					i++;
				}
				
				token_id.push_back(Token::word);//push the acutal word like "hello world"
				tokens.push_back(std::move(remember));
				i--;//leftover i++ from the while(){}
			
			} else if (isdigit(static_cast<unsigned char>(ch))){
				remember+=ch;
				i++;
				while(i<text.size() && isdigit(static_cast<unsigned char>(text[i]))){
					remember+=text[i];
					i++;
				}
				tokens.push_back(std::move(remember));
				token_id.push_back(Token::number);
				i--;
			} else if (ispunct(static_cast<unsigned char>(ch))){
				remember+=ch;
				tokens.push_back(remember);
				remember.clear();
				switch (ch) {
						//syntax
					case ';':
						token_id.push_back(Token::semicolon);
					break;
					case ':':
						token_id.push_back(Token::colon);
					break;
					case '"':
						token_id.push_back(Token::quote);
						i++;
						while(i<text.size()&&static_cast<unsigned char>(text[i])!='"'){
							remember+=text[i];
							i++;
						}
						if (!remember.empty()){
							token_id.push_back(Token::word);
							tokens.push_back(remember);	
						}
						if(i==text.size()-1){
							throw std::runtime_error("Reached EOF but a quote for a string never ended");
						}
						token_id.push_back(Token::quote);
						tokens.push_back("\"");
					break;
					case '.':
						token_id.push_back(Token::dot);
					break;
					case ',':
						token_id.push_back(Token::comma);
					break;
						//opeators
					case '=':
						token_id.push_back(Token::equal);
					break;
						//brackets
					case '{':
						token_id.push_back(Token::open_curly_bracket);
					break;
					case '}':
						token_id.push_back(Token::closed_curly_bracket);
					break;
					case '[':
						token_id.push_back(Token::open_square_bracket);
					break;
					case ']':
						token_id.push_back(Token::closed_square_bracket);
					break;
					case '(':
						token_id.push_back(Token::open_round_bracket);
					break;
					case ')':
						token_id.push_back(Token::closed_round_bracket);
					break;
					default:
						token_id.push_back(Token::other);
					break;
				}
			}
		}
	}
