#include <algorithm>
#include <arpa/inet.h>
#include <csignal>
#include <unistd.h>
#include <sys/wait.h>
#include "cgiControler.hpp"
#include "main.hpp"
#include "main.tpp"
#include "RequestProcessor.hpp"
#include "TCPServer.hpp"

bool	TCPServer::_close_server = false;

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
	// log_info("TCPServer destroyed");
}

void	TCPServer::signal_handler(int signum) {
	if (signum == SIGINT) {
		log_info("SIGINT received, shutting down server...");
		_close_server = true;
	}
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
	// log_info("Waiting for events...");
	_poller.wait(-1, ready_fds);
	if (_close_server) {
			log_info("Server is shutting down, ignoring events");
			return 1;
	}
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
		else if (is_a_cgi(*it)) {
			// log_warning<int>("The fd must be a pipe: ", *it);
			st = handle_cgi_event(*it);
		}
		else {
			log_warning<int>("Unknown fd ready: ", *it);
			log_error("VERY FATAL ERROR...");
			continue ;
		}
		if (st == CLIENT_DISCONNECTED) {
			log_info("Client disconnected");
			close_client_connection("Client disconnected ", *it);
		}
		else if (st == EXECVE_FAILURE) {
			log_error("FATAL ERROR : Failed execve");
			return EXECVE_FAILURE;
		}
	}
	return 0;
}

void	TCPServer::close_client_connection(std::string msg, int fd) {
	log_warning<int>("Closing client connection. Cause: " + msg, fd);
	_poller.remove(fd);
	_client_ptr->clean_fd();
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

int	TCPServer::is_a_cgi(const int fd) {
	for (std::vector<CGIControler>::iterator it = _cgis.begin(); it != _cgis.end(); ++it) {
		if (it->get_input_w_pipe() == fd || it->get_output_r_pipe() == fd) {
			_cgi_control_ptr = &(*it);
			return 1;
		}
	}
	return 0;
}

int		TCPServer::add_new_client(void) {
	struct sockaddr_in	client_address;
	int client_fd = _socket_ptr->socket_accept(client_address);
	if (client_fd == -1) {
		log_error("Failed to accept new client connection");
		return -1;
	}
	ClientConnection new_client(client_fd);
	const serverConfig * config = find_server_config(Socket::get_port(client_fd));
	if (!config) {
		log_error("Client is refused");
		close(client_fd);
		return -1;
	}
	_clients.push_back(new_client);
	_clients.back().setServerConfig(config);
	_poller.add(client_fd, POLLIN);
	log_info<std::string>("Added new client");
	return 0;
}

const serverConfig *	TCPServer::find_server_config(int port) {
	for (std::set<serverConfig>::iterator it = _servers.begin(); it != _servers.end(); ++it) {
		if (it->port == port)
			return &(*it);
	}
	log_error("No server config found for port");
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
		status = _client_ptr->send_response();
		if (status == WAITING) {
			try { 
				if (_client_ptr->get_headers().at("Connection") == "keep-alive")
					_poller.modify(fd, POLLIN);
				else
					status = CLOSING;
			}
			catch (std::out_of_range & e) { close_client_connection("Close in header", fd); }
		}
	}
	if (status == CLOSING || status == RECV_FAILURE || status == SEND_FAILURE) {
		log_info("Taking down client connection");
		return CLIENT_DISCONNECTED;
	}
	else if (status == BUILDING_RESPONSE) {
		log_info("Building response for client");
		if (build_client_response(status) == EXECVE_FAILURE) {
			return EXECVE_FAILURE;
		}
	}
	if (status == SENDING_RESPONSE) {
		log_info("Sending response to client");
		_poller.modify(fd, POLLOUT);
	}
	return SUCCESS;
}

exit_status	TCPServer::build_client_response(client_status &status) {
	_client_ptr->setLocationConfig();

	RequestProcessor	rq;
	switch (rq.process_request(*_client_ptr)) {
		case REDIRECTION:
			log_info("Redirect detected");
			status = _client_ptr->prepare_redirect(_client_ptr->get_uri());
			break ;
		case CGI_REQUEST:
			log_info("CGI detected");
			return prepare_cgi_process(_client_ptr->getFd());
		case POST_REQUEST:
			log_info("Upload detected");
			status = _client_ptr->prepare_post(_client_ptr->get_uri(), _client_ptr->get_body(), _client_ptr->getContentType());
			break ;
		case DELETE_REQUEST:
			log_info("Delete detected");
			status = _client_ptr->prepare_delete(_client_ptr->get_uri());
			break ;
		case STATIC_FILE:
			log_info("Static file detected");
			status = _client_ptr->prepare_get(_client_ptr->get_uri());
			break ;
		case NOT_ALLOWED:
			log_info("Unknown request type detected");
			_client_ptr->prepare_error_response(405);
			status = SENDING_RESPONSE;
			break ;
	}
	_client_ptr->finalize_response(_client_ptr->getConnectionHeader());
	_poller.modify(_client_ptr->getFd(), POLLOUT);
	return SUCCESS;
}

exit_status	TCPServer::prepare_cgi_process(const int fd) {
	CGIControler cgi(fd);
	if (cgi.initiate_cgi(*_client_ptr) == PIPE_FAILURE) {
		log_error("Failed to initiate CGI process");
		_client_ptr->prepare_error_response(500);
		_poller.modify(_client_ptr->getFd(), POLLOUT);
		return PIPE_FAILURE;
	}
	pid_t cgi_pid = cgi.fork_dup_op(*_client_ptr);
	if (cgi_pid == -1) {
		log_error("Failed to fork CGI process");
		_client_ptr->prepare_error_response(500);
		_poller.modify(_client_ptr->getFd(), POLLOUT);
		return FORK_FAILURE;
	}
	if (cgi_pid == 0) { // child process
		cgi.build_envp(*_client_ptr, *_cgi_config_ptr);
		return cgi.execute_cgi();
	}
	else { // parent process
		if (cgi.get_input_w_pipe() != -1) {
			_poller.add(cgi.get_input_w_pipe(), POLLOUT);
		}
		_poller.add(cgi.get_output_r_pipe(), POLLIN);
		_cgis.push_back(cgi);
		log_info("CGI process initiated successfully");
	}
	return SUCCESS;
}

exit_status	TCPServer::handle_cgi_event(int fd) {
	is_a_client(_cgi_control_ptr->get_client_fd());
	time_t start = _cgi_control_ptr->get_start_time();
	time_t current = std::time(NULL);
	if (current - start >= TIMEOUT) {
		log_warning<std::string>("CGI process timed out: ", _cgi_control_ptr->get_exec_path());
		kill_cgi(504);
	}
	short events = _poller.getRevents(fd);
	if (events & (POLLERR | POLLNVAL)) {
		log_warning<std::string>("CGI process has error or disconnected: ", _cgi_control_ptr->get_exec_path());
		kill_cgi(500);
	}
	if (events & POLLHUP) {
		if (fd == _cgi_control_ptr->get_output_r_pipe()) {
			int exit_status;
			if (waitpid(_cgi_control_ptr->get_child_pid(), &exit_status, WNOHANG) != _cgi_control_ptr->get_child_pid()) {
				return SUCCESS; // Let's wait for cgi process to quit to see error code
			}
			if (WIFEXITED(exit_status)) {
				exit_status = WEXITSTATUS(exit_status);
			}
			if (exit_status == 0) { // Exit was correct we can read the output of the CGI
				_client_ptr->prepare_cgi(_cgi_control_ptr->get_received_data());
			}
			else 
				_client_ptr->prepare_error_response(exit_status);
			_client_ptr->finalize_response(_client_ptr->getConnectionHeader());
			_poller.modify(_client_ptr->getFd(), POLLOUT);
			close(fd);
		}
		else if (fd == _cgi_control_ptr->get_input_w_pipe()) {
			close(fd);
			kill_cgi(500);
			// The input pipe should only be close by server when all data transmit. 
			// If this happens: this is an error -> so kill cgi and send 500 to client
		}
		_cgis.erase(std::remove(_cgis.begin(), _cgis.end(), *_cgi_control_ptr));
		_poller.remove(fd);
	}
	else if (events & POLLIN) {
		_cgi_control_ptr->cgi_received_data();
	}
	else if (events & POLLOUT) {
		if (_cgi_control_ptr->feed_cgi_process() == true) {
			_poller.remove(_cgi_control_ptr->get_input_w_pipe());
			close(_cgi_control_ptr->get_input_w_pipe());
		}
	}
	return SUCCESS;
}

void	TCPServer::kill_cgi(const int error_code) {
	kill(_cgi_control_ptr->get_child_pid(), SIGKILL);
	_poller.remove(_cgi_control_ptr->get_input_w_pipe());
	_poller.remove(_cgi_control_ptr->get_output_r_pipe());
	_client_ptr->prepare_error_response(error_code);
}
