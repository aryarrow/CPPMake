#pragma once
#include <ostream>
enum class Token{
	word,// example: quotation, word, quotaion = "hi"
	number,
	//basics for syntax
	semicolon,
	colon,
	dot,
	comma,
	quote,
	//operators
	equal,
	//brackets
	open_square_bracket,
	closed_square_bracket,
	
	open_round_bracket,
	closed_round_bracket,
	
	open_curly_bracket,
	closed_curly_bracket,
	//others that are not on this list like + / \ which i dont need these will bring an automatic error
	//cause they're not needed and use case is rare
	other
};

std::ostream& operator<<(std::ostream& stream, Token token);
