#ifndef SOCKET_HPP
# define SOCKET_HPP

# include <arpa/inet.h>
# include <string>

# include "main.hpp"

class Socket {
	public:
		Socket(void);
		Socket(const Socket &other);
		Socket&	operator=(const Socket &other);
		~Socket(void);

		socket_status	init(std::string ip, int port);
		int	getSockfd(void) const;
	
	private:
		int				created_socket(void);
		socket_status	bind_socket(std::string ip, int port);
		socket_status	listen_socket(void);

		socket_status	set_socket_nonblock(void);

		in_addr_t	inet_addr(const char *cp);

	private:
		int _sockfd;
};

#endif
