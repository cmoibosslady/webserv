#include "config.hpp"
#include "tokeniser.hpp"
#include "stoul.hpp"

static int	consumeSemicolon(Tokeniser &tokeniser) {
	if (tokeniser.getNextToken().type != TOKEN_SEMICOLON) {
		log_error<std::string>("Expected ';' at the end of directive" + tokeniser.getLineContext());
		return -1;
	}
	return 0;
}

int config::addPort(Tokeniser &tokeniser, serverConfig &server) const {
	conf_token token = tokeniser.getNextToken();
	if (token.type != TOKEN_WORD) {
		log_error<std::string>("Expected a port number, got: " + token.value + tokeniser.getLineContext());
		return -1;
	}
	int port = ft_stoul(token.value);
	if (port < 1 || port > 65535) {
		log_error<std::string>("Invalid port number: " + token.value + tokeniser.getLineContext());
		return -1;
	}
	server.port = port;
	return consumeSemicolon(tokeniser);
}

int config::addErrorPage(Tokeniser &tokeniser, serverConfig &server) const {
	conf_token token = tokeniser.getNextToken();
	if (token.type != TOKEN_WORD) {
		log_error<std::string>("Expected an error code, got: " + token.value + tokeniser.getLineContext());
		return -1;
	}
	int error_code = ft_stoul(token.value);
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
	return consumeSemicolon(tokeniser);
}

int	config::addClientMaxBodySize(Tokeniser &tokeniser, serverConfig &server) const {
	conf_token token = tokeniser.getNextToken();
	if (token.type != TOKEN_WORD) {
		log_error<std::string>("Expected a size value, got: " + token.value + tokeniser.getLineContext());
		return -1;
	}
	size_t size = std::stoul(token.value);
	server.client_max_body_size = size;
	return consumeSemicolon(tokeniser);
}

int	config::addAllowedMethods(Tokeniser &tokeniser, locationConfig &location) const {
	while (tokeniser.peek().type == TOKEN_WORD)
		location.allowed_methods.insert(tokeniser.getNextToken().value);
	if (location.allowed_methods.empty()) {
		log_error<std::string>("Expected at least one HTTP method. " + tokeniser.getLineContext());
		return -1;
	}
	return consumeSemicolon(tokeniser);
}

int	config::addRoot(Tokeniser &tokeniser, locationConfig &location) const {
	conf_token token = tokeniser.getNextToken();
	if (token.type != TOKEN_WORD) {
		log_error<std::string>("Expected a directory path, got: " + token.value + tokeniser.getLineContext());
		return -1;
	}
	location.root = token.value;
	return consumeSemicolon(tokeniser);
}

int	config::addAutoindex(Tokeniser &tokeniser, locationConfig &location) const {
	conf_token token = tokeniser.getNextToken();
	if (token.type != TOKEN_WORD || (token.value != "on" && token.value != "off")) {
		log_error<std::string>("Expected 'on' or 'off', got: " + token.value + tokeniser.getLineContext());
		return -1;
	}
	location.autoindex = (token.value == "on");
	return consumeSemicolon(tokeniser);
}

int	config::addIndex(Tokeniser &tokeniser, locationConfig &location) const {
	while (tokeniser.peek().type == TOKEN_WORD)
		location.index_files.insert(tokeniser.getNextToken().value);
	if (location.index_files.empty()) {
		log_error<std::string>("Expected at least one index file. " + tokeniser.getLineContext());
		return -1;
	}
	return consumeSemicolon(tokeniser);
}

int	config::addUploadAuth(Tokeniser &tokeniser, locationConfig &location) const {
	conf_token token = tokeniser.getNextToken();
	if (token.type != TOKEN_WORD || (token.value != "on" && token.value != "off")) {
		log_error<std::string>("Expected 'on' or 'off', got: " + token.value + tokeniser.getLineContext());
		return -1;
	}
	location.upload_auth = (token.value == "on");
	return consumeSemicolon(tokeniser);
}

int	config::addRewrite(Tokeniser &tokeniser, locationConfig &location) const {
	conf_token token = tokeniser.getNextToken();
	if (token.type != TOKEN_WORD) {
		log_error<std::string>("Expected a status code, got: " + token.value + tokeniser.getLineContext());
		return -1;
	}
	int status_code = ft_stoul(token.value);
	if (status_code < 300 || status_code > 399) {
		log_error<std::string>("Invalid redirect status code: " + token.value + tokeniser.getLineContext());
		return -1;
	}
	token = tokeniser.getNextToken();
	if (token.type != TOKEN_WORD) {
		log_error<std::string>("Expected a URL, got: " + token.value + tokeniser.getLineContext());
		return -1;
	}
	std::pair<int, std::string> redir(status_code, token.value);
	location.redirects[status_code] = token.value;
	return consumeSemicolon(tokeniser);
}
