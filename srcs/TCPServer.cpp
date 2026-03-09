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
		
	}
}
