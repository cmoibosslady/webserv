#include <unistd.h>
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

int	config::addUploadPath(Tokeniser &tokeniser, locationConfig &location) const {
	conf_token token = tokeniser.getNextToken();
	if (token.type != TOKEN_WORD) {
		log_error<std::string>("Expected a directory path, got: " + token.value + tokeniser.getLineContext());
		return -1;
	}
	location.upload_path = token.value;
	return consumeSemicolon(tokeniser);
}

int	config::addRewrite(Tokeniser &tokeniser, locationConfig &location) const {
	conf_token token = tokeniser.getNextToken();
	if (token.type != TOKEN_WORD && tokeniser.peek().type != TOKEN_WORD && tokeniser.doublePeek().type != TOKEN_WORD) {
		log_error<std::string>("Expected a rewrite rule, got: " + token.value + tokeniser.getLineContext());
		return -1;
	}
	if (token.value.substr(0, 1) != "^/" || token.value[token.value.size() - 1] != '$') {
		log_error<std::string>("Rewrite rule must be ^/<path>$ " + tokeniser.getLineContext());
		return -1;
	}
	struct rewriteConfig rule;
	rule.pattern = token.value.substr(2, token.value.size() - 1);
	rule.replacement = tokeniser.getNextToken().value;
	location.rewrites.insert(rule);
	return consumeSemicolon(tokeniser);
}

//line exemple: cgi .py /usr/bin/python3 POST;
int	config::addCgi(Tokeniser &tokeniser, locationConfig &location) const {
	conf_token token = tokeniser.getNextToken();
	if (token.type != TOKEN_WORD) {
		log_error<std::string>("Expected a file extension, got: " + token.value + tokeniser.getLineContext());
		return -1;
	}
	std::string extension = token.value;
	token = tokeniser.getNextToken();
	if (token.type != TOKEN_WORD) {
		log_error<std::string>("Expected a CGI script path, got: " + token.value + tokeniser.getLineContext());
		return -1;
	}
	std::string script_path = token.value;
	if (checkFile(script_path.c_str(), F_OK | X_OK) == -1) {
		log_error<std::string>("CGI script not found or not executable: " + script_path + tokeniser.getLineContext());
		return -1;
	}
	std::set<std::string> methods;
	while (tokeniser.peek().type == TOKEN_WORD)
		methods.insert(tokeniser.getNextToken().value);
	struct cgiConfig cgi;
	cgi.extension = extension;
	cgi.script_path = script_path;
	cgi.allowed_methods = methods;
	location.cgi_configs.insert(cgi);
	return consumeSemicolon(tokeniser);
}
