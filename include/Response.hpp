#ifndef RESPONSE_HPP
# define RESPONSE_HPP

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
		bool				build_response(const std::string & content, const int http_code, const std::string content_type = "text/plain; charset=utf-8");
		std::string			get_content_type(const std::string & file_path) const;
		client_status		send_response(void);

	private:
		std::string			_response;
		int					_fd;
};

#endif
