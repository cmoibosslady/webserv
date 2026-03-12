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

	protected:
		bool				build_response(const std::string content);
		client_status		send_response(void);

	private:
		std::string			_response;
		int					_fd;
};

#endif
