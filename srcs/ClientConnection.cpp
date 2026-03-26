#include <sys/socket.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include "ClientConnection.hpp"
#include "ft_mem.hpp"
#include "main.hpp"
#include "main.tpp"

ClientConnection::ClientConnection(void): Parser(), Response(-1), _fd(-1), _buffer(""), _status(WAITING), _lastActivity(std::time(NULL)), _server(0), _location(0) {
	// log_info("ClientConnection created");
}

ClientConnection::ClientConnection(int fd): Parser(), Response(fd), _fd(fd), _buffer(""), _status(WAITING), _lastActivity(std::time(NULL)), _server(0), _location(0) {
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

void	ClientConnection::setLocationConfig() {
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

std::string	ClientConnection::getLocationRoot() const {
	if (_location && !_location->root.empty())
		return _location->root;
	return "";
}

void	ClientConnection::setBuffer(const std::string cgi_answer) {
	if (!cgi_answer.empty()) {
		_buffer = cgi_answer;
	}
}

request_type	ClientConnection::find_type_request(void) {
	if (!_location)
		return UNKNOWN;
	std::string uri = get_uri().substr(get_uri().find(_location->path) + _location->path.size());
	
	return UNKNOWN;
}

bool	ClientConnection::needs_redirect(void) {
	std::string	uri = get_uri().substr(get_uri().find(_location->path) + _location->path.size());
	
	for (std::set<rewriteConfig>::const_iterator it = _location->rewrites.begin(); it != _location->rewrites.end(); ++it) {
		log_debug<std::string>("Uri=" + uri, "pattern="+it->pattern);
		if (uri == it->pattern) {
			log_debug<std::string>("Replace by", it->replacement);
			_rewrite = &(*it);
			return true;	
		}
	}
	_rewrite = NULL;
	return false;
}

const cgiConfig *	ClientConnection::needs_cgi(void) const {
	if (!_location || _location->cgi_configs.empty()) {
		log_debug<std::string>("No cgi config found for URI: ", get_uri());
		return NULL;
	}
	if (get_uri().find('.') == std::string::npos) {
		log_debug<std::string>("No CGI dot extension founded for URI: ", get_uri());
		return NULL;
	}
	std::string	uri_extension = get_uri().substr(get_uri().find_last_of('.'), get_uri().find_last_of('?') - get_uri().find_last_of('.'));
	log_debug<std::string>("URI extension: ", uri_extension);
	for (std::set<cgiConfig>::const_iterator it = _location->cgi_configs.begin(); it != _location->cgi_configs.end(); ++it) {
		log_debug<std::string>("Checking CGI extension: ", it->extension);
		if (it->extension == uri_extension) {
			if (it->allowed_methods.find(get_method()) != it->allowed_methods.end())
				return &(*it);
		}
	}
	log_debug<std::string>("No CGI needed for URI: ", get_uri());
	
	return NULL;
}

bool	ClientConnection::needs_static_response(void) {
	if (get_method() != "GET")
		return false;
	return true;
}

int		ClientConnection::get_on_file(void) {
	if (!_location || _location->root.empty()) {
		_status = prepareResponse(404);
		return _status;
	}
	std::string uri_path = get_uri().substr(0, get_uri().find('?')); // on enleve la query '? et apres'
	std::string relative_path = uri_path;
	if (uri_path.find(_location->path) == 0)
		relative_path = uri_path.substr(_location->path.size());
	while (!relative_path.empty() && relative_path[0] == '/')
		relative_path.erase(0, 1);
	std::string file_path = _location->root;
	if (!file_path.empty() && file_path[file_path.size() - 1] != '/')
		file_path += '/';
	file_path += relative_path;
	if (access(file_path.c_str(), F_OK) != 0)
		_status = prepareResponse(404);
	else if (access(file_path.c_str(), R_OK) != 0)
		_status = prepareResponse(403);
	else
		_status = prepareResponse(200);
	return _status;
}


client_status	ClientConnection::processTransmit(void) {
	log_info("Processing client data");
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
	// log_debug<std::string>("Received data: ", _buffer);
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


client_status	ClientConnection::prepareResponse(request_type rq) {
	log_info("Connection status: " + get_headers().at("Connection"));
	if (http_status_code != 0) {
		if (http_status_code == 300) {
			log_info("Preparing response with content->redirect");
			content = _rewrite->replacement;
			log_debug<std::string>("Rewrite replacement", _rewrite->replacement);
			http_status_code = _rewrite->error_code;
			build_redirect(get_uri(), _location->path, _rewrite->replacement);
			add_to_headers("Content-Type", "text/plain; charset=utf-8");
		}
		else if (content.empty() == false) {
			log_info("Preparing response with content->CGI");
			_buffer = content;
			http_status_code = 200;
			add_to_headers("Content-Type", "text/html; charset=utf-8");
		}
	}
	else if (get_method() == "GET" || get_method() == "HEAD") {
		log_info("Preparing response with content->GET or HEAD");
		// search for file
		_buffer = "File path: " + get_uri() + "\n";
		add_to_headers("Content-Type", "text/plain; charset=utf-8");
	}
	else {
		log_info("Preparing response with empty content");
		add_to_headers("Content-Type", "text/plain; charset=utf-8");
	}
	// before call function to set _buffer to correct content, then send buffer
	
	build_response(_buffer, http_status_code, get_headers().at("Connection"));
	_status = SENDING_RESPONSE;
	_buffer.clear();
	return _status;
}

client_status	ClientConnection::send_response(void) {
	_status = Response::send_response();
	return _status;
}
