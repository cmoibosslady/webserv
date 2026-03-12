#ifndef PARSER_HPP
# define PARSER_HPP

# include <map>
# include <string>
# include "main.hpp"

class Parser {
	public:
		Parser();
		Parser(Parser const & other);
		Parser & operator=(Parser const & other);
		~Parser();

	protected:
		client_status	parse_request_line(const std::string & line);
		client_status	parse_header(const std::string & raw_header);
		client_status	parse_body(const std::string & body);

		bool			check_chunk(void);
		bool 			unchunk_body(void);
	
		const std::string	&							get_method(void) const;
		const std::string	&							get_uri(void) const;
		
		const std::map<std::string, std::string> &	get_headers(void) const;
		const std::string	&							get_body(void) const;

	private:
		std::string								_method;
		std::string								_uri;
		std::string								_version;

		std::map<std::string, std::string>		_headers;
		std::string								_body;
};

#endif
