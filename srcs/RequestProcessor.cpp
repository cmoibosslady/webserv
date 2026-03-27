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
	std::string method = client.get_method();
}
