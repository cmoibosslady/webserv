#include <sstream>
#include <stoul.hpp>
#include "Parser.hpp"

Parser::Parser(void) {
	// log_info("Parser constructor called");
}

Parser::Parser(Parser const & other) {
	// log_info("Parser copy constructor called");
	*this = other;
}

Parser & Parser::operator=(Parser const & other) {
	// log_info("Parser assignment operator called");
	this->_method = other._method;
	this->_uri = other._uri;
	this->_version = other._version;
	this->_headers = other._headers;
	this->_body = other._body;
	return *this;
}

Parser::~Parser(void) {
	// log_info("Parser destructor called");
}

// GETTERS

const std::string &	Parser::get_method(void) const {
	return this->_method;
}

const std::string &	Parser::get_uri(void) const {
	return this->_uri;
}

const std::map<std::string, std::string> &	Parser::get_headers(void) const {
	return this->_headers;
}

const std::string &	Parser::get_body(void) const {
	return this->_body;
}

//For next two functions, line is already correct: it means we reached the \r\n or \r\n\r\n
client_status	Parser::parse_request_line(const std::string & line) {
	log_info("Parsing request line: " + line);
	std::stringstream ss(line);
	std::getline(ss, _method, ' ');
	std::getline(ss, _uri, ' ');
	std::getline(ss, _version);
	return READING_HEADER;
}

client_status	Parser::parse_header(const std::string & raw_header) {
	log_info("Parsing header line: " + raw_header);
	std::string key, value;
	std::stringstream ss(raw_header);
	while (std::getline(ss, key, ':')) {
		if (std::getline(ss, value)) {
			_headers[key] = value;
		}
		else {
			log_warning("Malformed header line: ", raw_header);
			return BAD_REQUEST; // stop parsing the header, flush the socket, send 400 Bad Request
		}
	}
	return READING_BODY;
}

// This function, the body is not necessarily complete
client_status	Parser::parse_body(const std::string & body) {
	log_info("Parsing body: " + body);
	_body.append(body);
	if (body.find("\r\n\r\n") != std::string::npos) {
		log_info("End of body reached");
		return SENDING_RESPONSE;
	}
	return READING_BODY;
}

// For chunk content-> transformation of the body happens rigth before receiving the body

bool	Parser::check_chunk(void) {
	log_info("Checking if body is chunked");
	std::map<std::string, std::string>::const_iterator it = _headers.find("Transfer-Encoding");
	if (it != _headers.end() && it->second.find("chunked") != std::string::npos) {
		return true;
	}
	return false;
}

bool	Parser::unchunk_body(void) {
	std::stringstream ss(_body);
	std::string chunk_size_str;
	size_t	chunk_size;
	_body.clear();
	while (std::getline(ss, chunk_size_str)) {
		try {chunk_size = ft_stoul(chunk_size_str, 0, 16);}
		catch (const std::exception & e) { return false; }
		if (chunk_size == 0) {
			break; // End of chunks
		}
		std::string chunk_data;
		chunk_data.resize(chunk_size);
		ss.read(&chunk_data[0], chunk_size);
		if (ss.gcount() != static_cast<std::streamsize>(chunk_size)) {
			log_warning("Chunk size mismatch: expected ", chunk_size);
			return false; // Malformed chunk
		}
		_body.append(chunk_data);
		ss.ignore(2); // Ignore the trailing \r\n after the chunk data
	}
	// set in the header the total size
	return (true);
}
