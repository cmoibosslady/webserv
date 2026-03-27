#include <string>

#include "ClientConnection.hpp"
#include "main.hpp"
#include "RequestProcessor.hpp"

RequestProcessor::RequestProcessor(void) {
	log_info("Request Processor default constructor called");
}

RequestProcessor::RequestProcessor(const RequestProcessor & other) {
	log_info("Request Processor copy constructor called");
	*this = other;
}

RequestProcessor&	RequestProcessor::operator=(const RequestProcessor & other) {
	this->_res = other._res;
	return *this;
}

RequestProcessor::~RequestProcessor(void) {
	log_info("Request Processor destructor called");
}

processing_result	RequestProcessor::process_request(ClientConnection & client) {
	std::string	uri = client.get_uri();
	const locationConfig *	loc = client.getLocation(); // can be 0...
	
	if (loc == NULL)
		return NOT_ALLOWED;

	uri = uri.substr(uri.find(loc->path) + loc->path.size(), uri.find("?"));
	
	// find if a redirect
	std::set<rewriteConfig>::const_iterator it = std::find(loc->rewrites.begin(), loc->rewrites.end(), uri);
	if (it != loc->rewrites.end())
		return REDIRECTION;

	// find if a cgi
	std::string method = client.get_method();
	if (uri.find(".") != std::string::npos) {
		std::string extension = uri.substr(uri.find_last_of("."));
		std::set<cgiConfig>::const_iterator it = std::find(loc->cgi_configs.begin(), loc->cgi_configs.end(), extension);
		if (it != loc->cgi_configs.end() && std::find(it->allowed_methods.begin(), it->allowed_methods.end(), method) != it->allowed_methods.end())
			return CGI_REQUEST;
	}

	// find if allowed
	if (std::find(loc->allowed_methods.begin(), loc->allowed_methods.end(), method) == loc->allowed_methods.end())
		return NOT_ALLOWED;
}
