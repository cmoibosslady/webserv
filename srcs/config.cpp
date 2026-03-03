#include "config.hpp"
#include "main.hpp"
#include "tokeniser.hpp"

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

std::set<serverConfig> parseConfFile(void) const;
