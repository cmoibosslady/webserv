#include <fcntl.h>
#include<sys/socket.h>
#include <sstream>
#include <unistd.h>

#include "Socket.hpp"
#include "stoul.hpp"

const std::string Socket::_ip = "0.0.0.0";

Socket::Socket(void): _sockfd(-1) {
	// log_info("Socket instance created");
}

Socket::Socket(const Socket & other) {
	*this = other;
	// log_info("Socket instance copied");
}

Socket&	Socket::operator=(const Socket & other) {
	this->_sockfd = other._sockfd;
	return *this;
}

Socket::~Socket(void) {
	// log_info("Socket instance destroyed");
}

int Socket::getSockfd(void) const {
	return _sockfd;
}

int	Socket::closeSocket(void) {
	close(_sockfd);
	return 0;
}

socket_status Socket::init(int port) {
	_sockfd = created_socket();
	if (_sockfd < 0)
		return SOCKET_FAILURE;
	if (set_socket_nonblock() < 0)
		return SOCKET_FAILURE;
	if (bind_socket(_ip, port) < 0)
		return BIND_FAILURE;
	if (listen_socket() < 0)
		return LISTEN_FAILURE;
	return SOCKET_SUCCESS;
}

int Socket::created_socket(void) {
	_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	return _sockfd;
}

socket_status	Socket::bind_socket(const std::string ip, int port) {
	struct sockaddr_in	address;
	address.sin_family = AF_INET;
	address.sin_port = htonl(port);
	if (this->inet_aton(ip.c_str(), &address.sin_addr) == -1)
		return WRONG_IP_ADDR;
	if (bind(_sockfd, (struct sockaddr *)&address, sizeof(address)) == - 1)
		return BIND_FAILURE;
	return SOCKET_SUCCESS;
}

socket_status Socket::listen_socket(void) {
	if (listen(_sockfd, SOMAXCONN) == -1) // SOMAXCONN = maximum of pending connection in the queue of the socket
		return LISTEN_FAILURE;
	return SOCKET_SUCCESS;
}

socket_status Socket::set_socket_nonblock(void) {
	int flags = fcntl(_sockfd, F_GETFL, 0);
	if (flags == -1)
		return SOCKET_FAILURE;
	if (fcntl(_sockfd, F_SETFL, flags | O_NONBLOCK) == -1)
		return SOCKET_FAILURE;
	return SOCKET_SUCCESS;
}

int		Socket::socket_accept(void) {
	struct sockaddr_in	client_address;
	socklen_t			client_len = sizeof(client_address);
	int					client_fd = accept(_sockfd, (struct sockaddr *)&client_address, &client_len);
	return				client_fd;
}

int	Socket::inet_aton(const char *cp, struct in_addr *inp) {
	std::stringstream ss(cp);
	std::string token;
	unsigned long	result = 0;

	for (int i = 3; i >= 0; --i) {
		if (!std::getline(ss, token, '.'))
			return -1;
		unsigned long octet;
		try { octet = ft_stoul(token); }
		catch (const std::exception &e) { return -1; }
		if (octet > 255)
			return -1;
		result |= (octet << (i * 8));
	}
	inp->s_addr = htonl(result);
	return 0;
}
