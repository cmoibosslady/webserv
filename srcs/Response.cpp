#include <sstream>
#include <sys/socket.h>
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
	this->_response = other._response;
	return *this;
}

Response::~Response(void) {
	// log_info("Response destructor called");
}

bool Response::build_response(const std::string content) {
	// This function should build the response string based on the parsed request and the server's configuration.
	// For now, we will just set a simple response for testing purposes.
	std::stringstream ss;
	ss << "HTTP/1.1 200 OK\r\n" 
		<< "Content-Type: text/plain\r\n" 
		<< "Content-Length: " << content.size() << "\r\n"
		<< "\r\n" << content;
	_response = ss.str();
	return true;
}

client_status Response::send_response(void) {
	ssize_t	bytes_send = send(_fd, _response.c_str(), _response.size(), 0);
	if (bytes_send == static_cast<ssize_t>(_response.size()))
		return WAITING;
	size_t	by = static_cast<size_t>(bytes_send);
	_response.erase(0, by);
	return SENDING_RESPONSE;
}
