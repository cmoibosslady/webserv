#include "config.hpp"

bool	serverConfig::operator<(const serverConfig &other) const {
	return (this->port < other.port);
}

bool	locationConfig::operator<(const locationConfig &other) const {
	return (this->path < other.path);
}

bool	cgiConfig::operator<(const cgiConfig &other) const {
	return (this->extension < other.extension);
}

bool	rewriteConfig::operator<(const rewriteConfig &other) const {
	return (this->pattern < other.pattern);
}

