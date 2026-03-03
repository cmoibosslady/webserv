#ifndef TOKENISER_HPP
# define TOKENISER_HPP

# include <string>
# include "config.hpp"

enum config_file_token {
	TOKEN_WORD,
	TOKEN_SEMICOLON,
	TOKEN_LBRACE,
	TOKEN_RBRACE,
	TOKEN_COMMENT,
	TOKEN_EOF
};

struct conf_token {
	config_file_token	type;
	std::string 		value;
};

class Tokeniser {
	public:
		Tokeniser(void);
		Tokeniser(const Tokeniser & other);
		Tokeniser & operator=(const Tokeniser & other);
		~Tokeniser(void);
	
		void setInput(const std::string & input);
		std::string getLineContext(void) const;
		
		conf_token getNextToken(void);
		conf_token peek(void);
		conf_token doublePeek(void);

	private:
		std::string _input;
		size_t 		_pos;
		size_t		_line_number;
};

#endif
