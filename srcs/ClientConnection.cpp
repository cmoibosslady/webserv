#include <sys/socket.h>
#include <unistd.h>
#include "ClientConnection.hpp"
#include "ft_mem.hpp"
#include "main.hpp"
#include "main.tpp"

ClientConnection::ClientConnection(void): Parser(), Response(-1), _fd(-1), _buffer(""), _status(WAITING), _lastActivity(std::time(NULL)) {
	// log_info("ClientConnection created");
}

ClientConnection::ClientConnection(int fd): Parser(), Response(fd), _fd(fd), _buffer(""), _status(WAITING), _lastActivity(std::time(NULL)) {
	// log_info("ClientConnection created after connection accepted");
}

ClientConnection::ClientConnection(const ClientConnection& other): Parser(other), Response(other), _fd(other._fd), _buffer(other._buffer), _status(other._status), _lastActivity(other._lastActivity) {
	// log_info("ClientConnection copied");
}

ClientConnection&	ClientConnection::operator=(const ClientConnection& other) {
	_fd = other._fd;
	_buffer = other._buffer;
	_status = other._status;
	_lastActivity = other._lastActivity;
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
	if (_buffer.find("\r\n") != std::string::npos && _status == WAITING) {
		_status = parse_request_line(_buffer.substr(0, _buffer.find("\r\n")));
		_buffer.erase(0, _buffer.find("\r\n") + 2);
	}
	if (_buffer.find("\r\n\r\n") != std::string::npos && _status == READING_HEADER) {
		_status = parse_header(_buffer.substr(0, _buffer.find("\r\n\r\n")));
		_buffer.erase(0, _buffer.find("\r\n\r\n") + 4);
	}
	if (_buffer.size() > 0 && _status == READING_BODY) {
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
