#include "config.hpp"
#include "main.hpp"
#include "tokeniser.hpp"

#include <sys/stat.h>
#include <unistd.h>

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
	if (checkFile(config_file.c_str()))
		return -1;
	return 0;
}

int	config::checkFile(const char *__restrict__ file_path) const {
	if (access(file_path, F_OK | R_OK)) {
		log_error<std::string>("config file cannot be open");
		log_error<std::string>(strerror(errno));
		return -1;
	}
	struct stat st;
	if (stat(file_path,  &st) == -1) {
		log_error<std::string>("config file cannot be stat");
		log_error<std::string>(strerror(errno));
		return -1;
	}
	if (access(file_path, R_OK) == -1) {
		log_error<std::string>("config file cannot be read");
		log_error<std::string>(strerror(errno));
		return -1;
	}
	return 0;
}
