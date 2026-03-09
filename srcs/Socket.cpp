#include<sys/socket.h>

#include "Socket.hpp"

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

socket_status Socket::init(std::string ip, int port) {
	_sockfd = created_socket();
	if (_sockfd < 0)
		return SOCKET_FAILURE;
	if (set_socket_nonblock() < 0)
		return SOCKET_FAILURE;
	if (bind_socket(ip, port) < 0)
		return BIND_FAILURE;
	if (listen_socket() < 0)
		return LISTEN_FAILURE;
	return SOCKET_SUCCESS;
}

int Socket::created_socket(void) {
	_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	return _sockfd;
}

socket_status	Socket::bind_socket(std::string ip, int port) {
	struct sockaddr_in	address;
	address.sin_family = AF_INET;
	address.sin_port = htonl(port);
	if (inet_aton(ip.c_str(), &address.sin_addr) == -1)
		return WRONG_IP_ADDR;
	if (bind(_sockfd, (struct sockaddr *)&address, sizeof(address)) == - 1)
		return BIND_FAILURE;
	return SOCKET_SUCCESS;
}
