#include "config.hpp"
#include "tokeniser.hpp"

int config::addPort(Tokeniser &tokeniser, serverConfig &server) const {
	conf_token token = tokeniser.getNextToken();
	if (token.type != TOKEN_WORD) {
		log_error<std::string>("Expected a port number, got: " + token.value + tokeniser.getLineContext());
		return -1;
	}
	int port = std::stoi(token.value);
	if (port < 1 || port > 65535) {
		log_error<std::string>("Invalid port number: " + token.value + tokeniser.getLineContext());
		return -1;
	}
	server.port = port;
	return 0;
}

int config::addErrorPage(Tokeniser &tokeniser, serverConfig &server) const {
	conf_token token = tokeniser.getNextToken();
	if (token.type != TOKEN_WORD) {
		log_error<std::string>("Expected an error code, got: " + token.value + tokeniser.getLineContext());
		return -1;
	}
	int error_code = std::stoi(token.value);
	if (error_code < 400 || error_code > 599) {
		log_error<std::string>("Invalid error code: " + token.value + tokeniser.getLineContext());
		return -1;
	}
	token = tokeniser.getNextToken();
	if (token.type != TOKEN_WORD) {
		log_error<std::string>("Expected a file path, got: " + token.value + tokeniser.getLineContext());
		return -1;
	}
	server.error_pages[error_code] = token.value;
	return 0;
}

int	config::addClientMaxBodySize(Tokeniser &tokeniser, serverConfig &server) const {
	conf_token token = tokeniser.getNextToken();
	if (token.type != TOKEN_WORD) {
		log_error<std::string>("Expected a size value, got: " + token.value + tokeniser.getLineContext());
		return -1;
	}
	size_t size = std::stoul(token.value);
	server.client_max_body_size = size;
	return 0;
}

int	config::addAlloweMethods(Tokeniser &tokeniser, locationConfig &location) const {
	conf_token token = tokeniser.getNextToken();
	if (token.type != TOKEN_WORD) {
		log_error<std::string>("Expected a method name, got: " + token.value + tokeniser.getLineContext());
		return -1;
	}
	location.allowed_methods.insert(token.value);
	if (tokeniser.peekNextToken().type != TOKEN_WORD) {	
		break;
	}
	return 0;
}
