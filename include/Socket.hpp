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

		socket_status	init(int port);
		int	getSockfd(void) const;
	
	private:
		int				created_socket(void);
		socket_status	bind_socket(const std::string ip, int port);
		socket_status	listen_socket(void);

		socket_status	set_socket_nonblock(void);

		int		socket_accept(void);

		int	inet_aton(const char *cp, struct in_addr *inp);

	private:
		int 						_sockfd;
		const static std::string	_ip;
};

#endif
