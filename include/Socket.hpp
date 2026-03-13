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
		int				getSockfd(void) const;
		int				socket_accept(struct sockaddr_in &client_address) const;
		int				closeSocket(void);

		static int		get_port(int fd);
	
	private:
		int				created_socket(void);
		socket_status	bind_socket(const std::string ip, int port);
		socket_status	listen_socket(void);
		socket_status	set_socket_nonblock(void);

		int				inet_aton(const char *cp, struct in_addr *inp);

	private:
		int 						_sockfd;
		const static std::string	_ip;
};

#endif
