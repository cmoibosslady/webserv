#include <unistd.h>
#include "ClientConnection.hpp"
#include "main.hpp"

ClientConnection::ClientConnection(void): _fd(-1), _buffer(""), _status(WAITING), _lastActivity(std::time(NULL)) {
	// log_info("ClientConnection created");
}

ClientConnection::ClientConnection(int fd): _fd(fd), _buffer(""), _status(WAITING), _lastActivity(std::time(NULL)) {
	log_info("ClientConnection created after connection accepted");
}

ClientConnection::ClientConnection(const ClientConnection& other): _fd(other._fd), _buffer(other._buffer), _status(other._status), _lastActivity(other._lastActivity) {
	log_info("ClientConnection copied");
}

ClientConnection&	ClientConnection::operator=(const ClientConnection& other) {
	_fd = other._fd;
	_buffer = other._buffer;
	_status = other._status;
	_lastActivity = other._lastActivity;
	return *this;
}

ClientConnection::~ClientConnection(void) {
	close(_fd);
	log_info("ClientConnection destroyed");
}

int	ClientConnection::getFd(void) const {
	return _fd;
}

int	ClientConnection::getStatus(void) const {
	return _status;
}
