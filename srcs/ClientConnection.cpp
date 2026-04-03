#include <sys/socket.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
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
	Parser::operator=(other);
	Response::operator=(other);
	_fd = other._fd;
	_buffer = other._buffer;
	_status = other._status;
	_lastActivity = other._lastActivity;
	_server = other._server;
	_location = other._location;
	_rewrite = other._rewrite;
	_cgi = other._cgi;
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

const std::string &	ClientConnection::get_method(void) const {
	return Parser::get_method();
}

const std::string &	ClientConnection::get_uri(void) const {
	return Parser::get_uri();
}

const std::map<std::string, std::string> &	ClientConnection::get_headers(void) const {
	return Parser::get_headers();
}

const std::string &	ClientConnection::get_body(void) const {
	return Parser::get_body();
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
	log_debug<int>("Server port: ", _server->port);
	setErrorPages(_server->error_pages);
}

void	ClientConnection::setLocationConfig(void) {
	if (!_server)
		return;
	log_debug<int>("Server port: ", _server->port);
	std::string uri = get_uri().substr(0, get_uri().find('?'));
	while (!uri.empty()) {
		log_debug<std::string>("Trying to match location for URI: ", uri);
		for (std::set<locationConfig>::const_iterator it = _server->locations.begin(); it != _server->locations.end(); ++it) {
			if (uri == it->path) {
				_location = &(*it);
				log_debug<std::string>("Matched location: ", _location->path);
				return;
			}
		}
		size_t last_slash = uri.find_last_of('/');
		if (last_slash == std::string::npos)
			break;
		else {
			if (last_slash == uri.size() - 1) {
				uri.erase(last_slash);		
			}
			else
				uri.erase(last_slash + 1);
		}
	}
	_location = 0;
}

void	ClientConnection::setRedirect(const rewriteConfig & redirect) {
	_rewrite = &redirect;
}

void	ClientConnection::setCgiConfig(const cgiConfig & cgi) {
	_cgi = &cgi;
}

const cgiConfig &	ClientConnection::getCgiConfig(void) const {
	return *_cgi;
}

const locationConfig *	ClientConnection::getLocation(void) const {
	return _location;
}

std::string	ClientConnection::getLocationRoot(void) const {
	if (_location && !_location->root.empty())
		return _location->root;
	return "";
}

std::string	ClientConnection::getConnectionHeader(void) const {
	std::string co_status;
	try { co_status = get_headers().at("Connection"); }
	catch (std::exception &e) {	co_status = "close"; }
	return co_status;
}

std::string ClientConnection::getContentType(void) const {
	std::string content_type;
	try { content_type = get_headers().at("Content-Type"); }
	catch (std::exception &e) { content_type = ""; }
	return content_type;
}

void	ClientConnection::setBuffer(const std::string cgi_answer) {
	if (!cgi_answer.empty()) {
		_buffer = cgi_answer;
	}
}

client_status	ClientConnection::processTransmit(void) {
	log_info("Processing client data");
	char buffer[BUFFER_SIZE];
	ssize_t bytesRead = recv(_fd, buffer, BUFFER_SIZE, 0);
	if (bytesRead == 0)
		return CLOSING;
	if (bytesRead < 0) {
		log_error("Error reading from client socket");
		return RECV_FAILURE;
	}
	_buffer.append(buffer, static_cast<size_t>(bytesRead));
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
		if (get_body().size() > _server->client_max_body_size)
			return BUILDING_RESPONSE;
	}
	return _status;
}

client_status	ClientConnection::send_response(void) {
	_status = Response::send_response();
	return _status;
}
