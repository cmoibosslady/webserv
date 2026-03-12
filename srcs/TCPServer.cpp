#include <algorithm>
#include <unistd.h>
#include "cgiControler.hpp"
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
	exit_status st;
	_poller.wait(-1, ready_fds);
	if (ready_fds.empty()) {
		log_info("No events occurred within the timeout period");
		return 0;
	}
	// log_info("Processing events");
	for (std::vector<int>::const_iterator it = ready_fds.begin(); it != ready_fds.end(); ++it) {
		if (is_a_socket(*it)) {
			if (add_new_client() != -1)
				log_info("New client connection accepted");
			continue ;
		}
		else if (is_a_client(*it)) {
			st = handle_client_event(*it);
		}
		else {
			log_warning<int>("The fd must be a pipe", *it);
			// st = handle_cgi_events(*it);
			// if (st == EXECVE_FAILURE) {
				// log_error("FATAL ERROR : Failed execve");
				// return EXECVE_FAILURE;
		}
		if (st == CLIENT_DISCONNECTED) {
			log_info("Client disconnected");
			close_fd("Client disconnected", *it);
		}
		else if (st == EXECVE_FAILURE) {
			log_error("FATAL ERROR : Failed execve");
			return EXECVE_FAILURE;
		}
	}
	return 0;
}

void	TCPServer::close_fd(std::string msg, int fd) {
	log_warning<int>("Closing fd. Cause: " + msg, fd);
	_poller.remove(fd);
	_clients.erase(std::remove(_clients.begin(), _clients.end(), *_client_ptr)); // Check if does erase correctly the clients from the server vector
	_client_ptr = NULL;
	close(fd);
}

int		TCPServer::is_a_socket(int fd) {
	for (std::vector<Socket>::iterator it = _sockets.begin(); it != _sockets.end(); ++it) {
		if (it->getSockfd() == fd) {
			_socket_ptr = &(*it);
			return 1;
		}
	}
	return 0;
}

int		TCPServer::is_a_client(int fd) {
	for (std::vector<ClientConnection>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		if (it->getFd() == fd) {
			_client_ptr = &(*it);
			return 1;
		}
	}
	return 0;
}

int		TCPServer::add_new_client(void) {
	int client_fd = _socket_ptr->socket_accept();
	if (client_fd == -1) {
		log_error("Failed to accept new client connection");
		return -1;
	}
	ClientConnection new_client(client_fd);
	_clients.push_back(new_client);
	_poller.add(client_fd, POLLIN);
	return 0;
}

exit_status	TCPServer::handle_client_event(int fd) {
	_client_ptr->updateLastActivity();
	client_status status = WAITING;
	short revents = _poller.getRevents(fd);
	if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
		log_warning<int>("Client has error or disconnected", fd);
		return CLIENT_DISCONNECTED;
	}
	else if (revents & POLLIN) {
		status = _client_ptr->processTransmit();
	}
	else if (revents & POLLOUT) {
		status = _client_ptr->sendResponse();
	}
	if (status == CLOSING || status == RECV_FAILURE || status == SEND_FAILURE) {
		log_info("Taking down client connection");
		return CLIENT_DISCONNECTED;
	}
	else if (status == BUILDING_RESPONSE) {
		// associer config avec client -> routeur
		// if cgi needed
			// do cgi -> return if error on child and behave normal on parent
			// wait for output of cgi
			// build response with cgi output and send it
		// if no cgi -> build response send it
		if (_client_ptr->needs_cgi() == true) {
			CGIControler cgi;
			cgi.initiate_cgi(_client_ptr); // Should activate CGI/fork + add to poll
			if (fork_and_exec_cgi(cgi) == EXECVE_FAILURE) {
				log_error("Failed to fork and exec CGI");
				return EXECVE_FAILURE;
			}
			_cgis.push_back(cgi);
			_poller.add(cgi.getPipeReadEnd(), POLLIN);
			if (cgi.need_input() == true)
				_poller.add(cgi.getPipeWriteEnd(), POLLOUT);
		}
		_client_ptr->sendResponse();
		_poller.modify(fd, POLLOUT);
	}
	return SUCCESS;
}

