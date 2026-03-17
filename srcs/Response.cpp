#include <sstream>
#include <sys/socket.h>
#include "main.tpp"
#include "Response.hpp"

Response::Response(void) {
	// log_info("Response constructor called");
}

Response::Response(int fd) : _fd(fd) {
	// log_info("Response constructor with fd called");
}

Response::Response(const Response & other) {
	// log_info("Response copy constructor called");
	*this = other;
}

Response & Response::operator=(const Response & other) {
	// log_info("Response assignment operator called");
	this->_fd = other._fd;
	this->_response = other._response;
	return *this;
}

Response::~Response(void) {
	// log_info("Response destructor called");
}

void	Response::clean_fd(void) {
	_fd = -1;
}

bool Response::build_response(const std::string content) {
	// This function should build the response string based on the parsed request and the server's configuration.
	// For now, we will just set a simple response for testing purposes.
	std::stringstream ss;
	ss << "HTTP/1.1 200 OK\r\n" 
		<< "Content-Type: text/plain\r\n" 
		<< "Content-Length: " << content.size() << "\r\n"
		<< "\r\n" << content
		<< "\r\n";
	_response = ss.str();
	return true;
}

client_status Response::send_response(void) {
	log_warning<int>("Sending response to client fd ", _fd);
	if (_response.empty()) {
		log_error("Response is empty, nothing to send");
		return WAITING;
	}
	ssize_t	bytes_send = send(_fd, _response.c_str(), _response.size(), MSG_NOSIGNAL);
	if (bytes_send == -1) {
		log_error("Failed to send response to client");
		return WAITING;
	}
	if (static_cast<size_t>(bytes_send) == _response.size())
		return WAITING;
	size_t	by = static_cast<size_t>(bytes_send);
	_response.erase(0, by);
	return SENDING_RESPONSE;
}
