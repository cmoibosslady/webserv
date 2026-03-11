#include <unistd.h>
#include "main.hpp"
#include "main.tpp"
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
	for (std::vector<Socket>::iterator it = _sockets.begin(); it != _sockets.end(); ++it) {
		it->closeSocket();
	}
	for (std::vector<ClientConnection>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		it->closeConnection();
	}
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
	log_info("All server sockets initialized successfully");
	log_info("Adding server sockets to poller");
	for (std::vector<Socket>::iterator it = _sockets.begin(); it != _sockets.end(); ++it) {
		_poller.add(it->getSockfd(), POLLIN);
	}
	log_info("TCP Server initialized successfully");
	return 0;
}

int	TCPServer::wait(void) {
	std::vector<int> ready_fds;
	_poller.wait(TIMEOUT, ready_fds);
	if (ready_fds.empty()) {
		log_info("No events occurred within the timeout period");
		return 0;
	}
	log_info("Processing events");
	for (std::vector<int>::const_iterator it = ready_fds.begin(); it != ready_fds.end(); ++it) {
		short revents = _poller.getRevents(*it);
		if (revents | POLLERR || revents | POLLHUP) {
			close_fd("an error occured on fd: ", *it);
		}
	}
	return 0;
}

void	TCPServer::close_fd(std::string msg, int fd) {
	log_warning<int>("Closing fd. Cause: " + msg, fd);
	close(fd);
	_poller.remove(fd);
}
