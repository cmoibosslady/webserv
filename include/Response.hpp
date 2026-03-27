#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <map>
# include <sstream>
# include <string>

# include "config.hpp"
# include "main.hpp"

class Response {
	private:
		Response(void);

	public:
		Response(int fd);
		Response(const Response & other);
		Response & operator=(const Response & other);
		~Response(void);

		void	clean_fd(void);

		void	prepare_redirect(const std::string &uri);
		client_status	prepare_post(const std::string &body);
		client_status	prepare_delete(void);
		client_status	prepare_get(void);

		void			prepare_cgi(void);
		void			prepare_error_response(int http_code);

	protected:
		void			setErrorPages(const std::map<int, std::string> &error_pages);
		bool			build_response(const std::string content, const int http_code, std::string co_status);
		


		template<typename T>
			void 	add_to_headers(const std::string &key, const T & value) {
				_headers << key << ": " << value << "\r\n";
			}

		std::string		get_content_type(const std::string & file_path) const;
		client_status	send_response(void);	

		const serverConfig		* _server;
		const locationConfig	* _location;
		const cgiConfig			* _cgi;
		const rewriteConfig		*_rewrite;

	private:
		
		void	classic_http_hat(int http_code);
		void	prepare_error_content(int http_code);

		std::stringstream		_status_line; // careful same attributes inside parser..
		std::stringstream		_headers;
		std::stringstream		_response_body;
		std::string				_response;
		int						_fd;

		std::map<int, std::string>	_error_pages;
};

#endif
