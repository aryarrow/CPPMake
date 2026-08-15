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


std::ostream& operator<<(std::ostream& stream,Token token) {
	switch (token) {
		case Token::word:
			stream<< "word";
			break;
		case Token::number:
			stream<< "number";
			break;
		case Token::semicolon:
			stream<< "semicolon";
			break;
		case Token::colon:
			stream<< "colon";
			break;
		case Token::dot:
			stream<< "dot";
			break;
		case Token::comma:
			stream<< "comma";
			break;
		case Token::quote:
			stream<< "quote";
			break;
		case Token::equal:
			stream<< "equal";
			break;
		case Token::open_square_bracket:
			stream<< "open_square_bracket";
			break;
		case Token::closed_square_bracket:
			stream<< "closed_square_bracket";
			break;
		case Token::open_round_bracket:
			stream<< "open_round_bracket";
			break;
		case Token::closed_round_bracket:
			stream<< "closed_round_bracket";
			break;
		case Token::open_curly_bracket:
			stream<< "open_curly_bracket";
			break;
		case Token::closed_curly_bracket:
			stream<< "closed_curly_bracket";
			break;
		case Token::other:
			stream<< "other";
			break;
	}
	return stream;
}
