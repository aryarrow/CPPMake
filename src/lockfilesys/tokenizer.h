#pragma once

#include <vector>
#include "tokens.h"
class Tokenizer {
public:
	std::vector<std::string> tokens;
	std::vector<Token> token_id;

	void print_all_tokens();
	Tokenizer(const std::string& filename);
private:
	void tokenize(const std::string& text);
};
