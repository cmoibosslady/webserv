#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include "config.hpp"
#include "main.hpp"
#include "tokeniser.hpp"

// Canonical form + constructor with filename
config::config(void) {
	log_info<std::string>("config private default constructor called");
}

config::config(const config & other) {
	log_info<std::string>("config copy constructor called");
	*this = other;
}

config & config::operator=(const config & other) {
	this->_config_file = other._config_file;
	return *this;
}

config::~config(void) {
	log_info<std::string>("config destructor called");
}

config::config(const std::string & filename) {
	log_info<std::string>("config constructor called");
	_config_file = filename;
}

// methods to parse config file
int config::parseConfFile(std::set<serverConfig> & servers) const {
	if (checkFile(_config_file.c_str()) == -1)
		return -1;
	std::ifstream file(_config_file.c_str());
	if (!file.is_open()) {
		log_error<std::string>("failed to open config file");
		return -1;
	}
	std::string line;
	Tokeniser tokeniser;
	while (std::getline(file, line)) {
		tokeniser.setInput(line);
		conf_token	token = tokeniser.getNextToken();
		if (token.type == TOKEN_COMMENT || token.type == TOKEN_END)
			continue;
		else if (token.type != TOKEN_WORD) {
			log_error<std::string>("unexpected token in config file: " + token.value);
			return -1;
		}
		if (token.value == "server" && tokeniser.getNextToken().type == TOKEN_LBRACE) {
			serverConfig server;
			if (parseServerBloc(file, server) == -1)
				return -1;
			servers.insert(server);
		}
		else {
			log_error<std::string>("Expected brace after server keyword" + tokeniser.getLineContext());
			return -1;
		}
	}
	return 0;
}

int	config::checkFile(const char *__restrict__ file_path) const {
	if (access(file_path, F_OK | R_OK) == -1) {
		log_error<std::string>(strerror(errno));
		return -1;
	}
	struct stat st;
	if (stat(file_path,  &st) == -1) {
		log_error<std::string>(strerror(errno));
		return -1;
	}
	if (st.st_size == 0) {
		log_error<std::string>("File is empty");
		return -1;
	}
	if (!S_ISREG(st.st_mode)) {
		log_error<std::string>("File must be a regular disk file");
		return -1;
	}
	// File is a regular, readable, non-empty file of reasonable size
	return 0;
}

int	config::checkDirectory(const char *__restrict__ dir_path) const {
	if (access(dir_path, F_OK | R_OK | X_OK) == -1) {
		log_error<std::string>(strerror(errno));
		return -1;
	}
	struct stat st;
	if (stat(dir_path,  &st) == -1) {
		log_error<std::string>(strerror(errno));
		return -1;
	}
	if (!S_ISDIR(st.st_mode)) {
		log_error<std::string>("Path must be a directory");
		return -1;
	}
	// Directory is accessible and valid
	return 0;
}

int config::parseServerBloc(std::ifstream &ifs, serverConfig &server) const {
	std::string line;
	Tokeniser tokeniser;
	while (std::getline(ifs, line)) {
		tokeniser.setInput(line);
		conf_token token = tokeniser.getNextToken();
		if (token.type == TOKEN_COMMENT || token.type == TOKEN_END)
			continue;
		if (token.type != TOKEN_WORD && token.type != TOKEN_RBRACE) {
			log_error<std::string>("unexpected token in server block: " + token.value);
			return -1;
		}
		if (token.value == "listen") {
			if (addPort(tokeniser, server) == -1)
				return -1;
		}
		else if (token.value == "error_pages") {
			if (addErrorPage(tokeniser, server) == -1)
				return -1;
		}
		else if (token.value == "client_max_body_size") {
			if (addClientMaxBodySize(tokeniser, server) == -1)
				return -1;
		}
		else if (token.value == "location") {
			locationConfig location;
			if (tokeniser.peek().type != TOKEN_WORD) {
				log_error<std::string>("Expected path after location keyword" + tokeniser.getLineContext());
				return -1;
			}
			if (tokeniser.peek().value.empty()) {
				log_error<std::string>("Location path cannot be empty" + tokeniser.getLineContext());
				return -1;
			}
			location.path = tokeniser.getNextToken().value;
			if (parseLocationBloc(ifs, location) == -1)
				return -1;
			server.locations.insert(location);
		}
		if (token.type == TOKEN_RBRACE)
			return 0; // End of server block
	}
	log_error<std::string>("unexpected end of file while parsing server block" + tokeniser.getLineContext());
	return -1;
}

int config::parseLocationBloc(std::ifstream &ifs, locationConfig &location) const {
	std::string line;
	Tokeniser tokeniser;
	while (std::getline(ifs, line)) {
		tokeniser.setInput(line);
		conf_token token = tokeniser.getNextToken();
		if (token.type == TOKEN_COMMENT || token.type == TOKEN_END)
			continue;
		if (token.type != TOKEN_WORD && token.type != TOKEN_RBRACE) {
			log_error<std::string>("unexpected token in location block: " + token.value + tokeniser.getLineContext());
			return -1;
		}
		if (token.value == "root") {
			if (addRoot(tokeniser, location) == -1)
				return -1;
		}
		else if (token.value == "index") {
			if (addIndex(tokeniser, location) == -1)
				return -1;
		}
		else if (token.value == "autoindex") {
			if (addAutoindex(tokeniser, location) == -1)
				return -1;
		}
		else if (token.value == "allowed_methods") {
			if (addAllowedMethods(tokeniser, location) == -1)
				return -1;
		}
		else if (token.value == "rewrite") {
			if (addRewrite(tokeniser, location) == -1)
				return -1;
		}
		else if (token.value == "cgi") {
			if (addCgi(tokeniser, location) == -1)
				return -1;
		}
		else {
			log_error<std::string>("unexpected directive in location block: " + tokeniser.getLineContext());
			return -1;
		}
		if (token.type == TOKEN_RBRACE)
			return 0; // End of location block
	}
	log_error<std::string>("unexpected end of file while parsing location block" + tokeniser.getLineContext());
	return -1;
}

// After parsing, check if value are correct:
int	config::checkRoot(locationConfig &location) const {
	if (location.root.empty())
		return 0;
	if (checkDirectory(location.root.c_str()) == -1) {
		log_error<std::string>("Root directory not found: " + location.root);
		return -1;
	}
	return 0;
}

int	config::checkIndexFiles(locationConfig &location) const {
	std::set<std::string>::const_iterator it, ite = location.index_files.end();
	for (it = location.index_files.begin(); it != ite; ++it) {
		std::string path = location.root + "/" + *it;
		if (checkFile(path.c_str()) == -1) {
			log_error<std::string>("Index file not found: " + path);
			return -1;
		}
	}
	return 0;
}
