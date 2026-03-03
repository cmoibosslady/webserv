#include "tokeniser.hpp"
#include <iostream>
#include <sstream>

// Canonical class form

Tokeniser::Tokeniser(void) {
	std::cout << "Tokeniser created" << std::endl;
	this->_line_number = 0;
}

Tokeniser::Tokeniser(const Tokeniser & other) {
	std::cout << "Tokeniser copy created" << std::endl;
	*this = other;
}

Tokeniser::~Tokeniser(void) {
	std::cout << "Tokeniser destroyed" << std::endl;
}

Tokeniser &	Tokeniser::operator=(const Tokeniser & other) {
	this->_input = other._input;
	this->_pos = other._pos;
	this->_line_number = other._line_number;
	return *this;
}

// Getters and setters

void	Tokeniser::setInput(const std::string & input) {
	this->_input = input;
	this->_pos = 0;
	this->_line_number++;
}

std::string	Tokeniser::getLineContext(void) const {
	std::ostringstream context;
	context << "Line :" << this->_line_number;
	context << " Position :" << this->_pos << std::endl;
	return context.str();
}

// Tokeniser methods

conf_token	Tokeniser::getNextToken(void) {
	conf_token token;

	// skip whitespace
	while (_pos < _input.length() && std::isspace(_input[_pos]))
		this->_pos++;
	// eof
	if (_pos >= _input.length())
		return (token.type = TOKEN_EOF, token);

	char currentChar = _input[_pos];
	if (currentChar == '{') {
		_pos++;
		return (token.type = TOKEN_LBRACE, token);
	}
	else if (currentChar == '}') {
		_pos++;
		return (token.type = TOKEN_RBRACE, token);
	}
	else if (currentChar == ';') {
		_pos++;
		return (token.type = TOKEN_SEMICOLON, token);
	}
	else if (currentChar == '#') {
		_pos = _input.length(); // skip to end of line
		return (token.type = TOKEN_COMMENT, token);
	}
	else {
		size_t wordStart = _pos;
		while (_pos < _input.length() && !std::isspace(_input[_pos]) &&
			   _input[_pos] != '{' && _input[_pos] != '}' &&
			   _input[_pos] != ';' && _input[_pos] != '#')
			this->_pos++;
		token.type = TOKEN_WORD;
		token.value = _input.substr(wordStart, _pos - wordStart);
		return token;
	}
}

conf_token	Tokeniser::peek(void) {
	size_t temp = this->_pos;
	conf_token token = this->getNextToken();
	this->_pos = temp;
	return token;
}

conf_token Tokeniser::doublePeek(void) {
	size_t temp = this->_pos;
	this->getNextToken();
	conf_token token = this->getNextToken();
	this->_pos = temp;
	return token;
}
