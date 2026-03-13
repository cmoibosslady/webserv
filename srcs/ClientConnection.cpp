#include <sys/socket.h>
#include <unistd.h>
#include "ClientConnection.hpp"
#include "ft_mem.hpp"
#include "main.hpp"
#include "main.tpp"

ClientConnection::ClientConnection(void): Parser(), Response(-1), _fd(-1), _buffer(""), _status(WAITING), _lastActivity(std::time(NULL)) {
	// log_info("ClientConnection created");
	_server = 0;
	_location = 0;
}

ClientConnection::ClientConnection(int fd): Parser(), Response(fd), _fd(fd), _buffer(""), _status(WAITING), _lastActivity(std::time(NULL)) {
	// log_info("ClientConnection created after connection accepted");
	_server = 0;
	_location = 0;
}

ClientConnection::ClientConnection(const ClientConnection& other): Parser(other), Response(other), _fd(other._fd), _buffer(other._buffer), _status(other._status), _lastActivity(other._lastActivity) {
	// log_info("ClientConnection copied");
}

ClientConnection&	ClientConnection::operator=(const ClientConnection& other) {
	_fd = other._fd;
	_buffer = other._buffer;
	_status = other._status;
	_lastActivity = other._lastActivity;
	_server = other._server;
	_location = other._location;
	return *this;
}

ClientConnection::~ClientConnection(void) {
	// log_info("ClientConnection destroyed");
}

bool	ClientConnection::operator==(const ClientConnection& other) const {
	return _fd == other._fd;
}

int	ClientConnection::getFd(void) const {
	return _fd;
}

int	ClientConnection::closeConnection(void) {
	close(_fd);
	return 0;
}

int	ClientConnection::getStatus(void) const {
	return _status;
}

void	ClientConnection::updateLastActivity(void) {
	_lastActivity = std::time(NULL);
}

void	ClientConnection::setServerConfig(const serverConfig * config) {
	_server = config;
}

void	ClientConnection::setLocationConfig() {
	if (!_server)
		return;
	std::string uri = get_uri();
	while (!uri.empty()) {
		log_warning<std::string>("Trying to match location for URI: ", uri);
		for (std::set<locationConfig>::const_iterator it = _server->locations.begin(); it != _server->locations.end(); ++it) {
			if (uri == it->path) {
				_location = &(*it);
				return;
			}
		}
		size_t last_slash = uri.find_last_of('/');
		if (last_slash == std::string::npos)
			break;
		else {
			if (last_slash == uri.size() - 1) {
				uri.erase(last_slash + 1);		
			}
			else
				uri.erase(last_slash);
		}
	}
	_location = 0;
}


const cgiConfig *	ClientConnection::needs_cgi(void) const {
	if (!_location || !_location->cgi_configs.empty())
		return 0;
	std::string	uri_extension = get_uri().substr(get_uri().find_last_of('.'));
	for (std::set<cgiConfig>::const_iterator it = _location->cgi_configs.begin(); it != _location->cgi_configs.end(); ++it) {
		if (it->extension == uri_extension)

			return &(*it);
	}
	return 0;
}

client_status	ClientConnection::processTransmit(void) {
	char buffer[BUFFER_SIZE];
	ft_memset(buffer, 0, BUFFER_SIZE);
	ssize_t bytesRead = recv(_fd, buffer, BUFFER_SIZE - 1, 0);
	if (bytesRead == 0)
		return CLOSING;
	if (bytesRead < 0) {
		log_error("Error reading from client socket");
		return RECV_FAILURE;
	}
	_buffer += std::string(buffer);
	if (_status == WAITING && _buffer.find("\r\n") != std::string::npos) {
		_status = parse_request_line(_buffer.substr(0, _buffer.find("\r\n")));
		_buffer.erase(0, _buffer.find("\r\n") + 2);
	}
	if (_status == READING_HEADER && _buffer.find("\r\n\r\n") != std::string::npos) {
		_status = parse_header(_buffer.substr(0, _buffer.find("\r\n\r\n")));
		_buffer.erase(0, _buffer.find("\r\n\r\n") + 4);
	}
	if (_status == READING_BODY && _buffer.size() > 0) {
		_status = parse_body(_buffer);
		_buffer.clear();
		if (_status == BUILDING_RESPONSE && check_chunk()) {
			if (!unchunk_body()) {
				log_error("Failed to unchunk body");
				return BAD_REQUEST;
			}
		}
	}
	return _status;
}

client_status	ClientConnection::prepareResponse() {
	log_info("Preparing response");
	_buffer = "1. 2. 3. 4. 5. 6. 7.";
	build_response(_buffer);
	return SENDING_RESPONSE;
}

client_status	ClientConnection::send_response(void) {
	_status = Response::send_response();
	return _status;
}
