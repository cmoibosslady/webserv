#include "main.hpp"
#include "TCPServer.hpp"

TCPServer::TCPServer(void) {
	// log_info("TCPServer private constructor called");
}

TCPServer::TCPServer(const std::set<serverConfig> &serversConfig): _servers(serversConfig) {
	log_info("TCPServer created with config");
}

TCPServer::TCPServer(const TCPServer &other) {
	*this = other;
	// log_info("TCPServer copied");
}

TCPServer&	TCPServer::operator=(const TCPServer &other) {
	_servers = other._servers;
	_poller = other._poller;
	_sockets = other._sockets;
	_clients = other._clients;
	return *this;
}

TCPServer::~TCPServer(void) {
	log_info("TCPServer destroyed");
}

int	TCPServer::init(void) {
	for (std::set<serverConfig>::const_iterator	it = _servers.begin(); it != _servers.end(); ++it) {
		Socket	socket;
		if (socket.init(it->port) != SOCKET_SUCCESS) {
			log_error("Failed to initialize socket for server");
			return -1;
		}
		log_warning<int>("Server listening on port ", it->port);
		_sockets.push_back(socket);
	}
	log_info("TCP Server initialized successfully");
	return 0;
}
