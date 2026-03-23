#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <sstream>
# include <string>

# include "main.hpp"

class Response {
	private:
		Response(void);

	public:
		Response(int fd);
		Response(const Response & other);
		Response & operator=(const Response & other);
		~Response(void);

		void				clean_fd(void);

	protected:
		bool			build_response(const std::string content, const int http_code, std::string co_status);
		
		void			build_redirect(const std::string & uri, const std::string & loc_path, const std::string & replacement);

		void			add_to_headers(const std::string & key, const std::string & value);

		std::string		get_content_type(const std::string & file_path) const;
		client_status	send_response(void);	


	private:
		std::stringstream		_status_line; // careful same attributes inside parser..
		std::stringstream		_headers;
		std::stringstream		_response_body;
		std::string				_response;
		int						_fd;
};

#endif
