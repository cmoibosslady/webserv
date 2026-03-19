#include <unistd.h>
#include <sstream>
#include <cctype>
#include "cgiControler.hpp"
#include "main.tpp"

static std::string	trim_copy(const std::string & s) {
	std::string::size_type start = 0;
	std::string::size_type end = s.size();
	while (start < end && std::isspace(static_cast<unsigned char>(s[start])))
		++start;
	while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1])))
		--end;
	return s.substr(start, end - start);
}

static void	push_env(std::vector<std::string> & env, const std::string & key, const std::string & value) {
	env.push_back(key + "=" + value);
}

CGIControler::CGIControler(void) : _client_ptr(NULL) {
	// log_info("CGIControler constructor called");
	_input_pipe[0] = -1;
	_input_pipe[1] = -1;
	_output_pipe[0] = -1;
	_output_pipe[1] = -1;
}

CGIControler::CGIControler(const CGIControler & other) : _client_ptr(other._client_ptr) {
	// log_info("CGIControler copy constructor called");
	*this = other;
}

CGIControler & CGIControler::operator=(const CGIControler & other) {
	// log_info("CGIControler assignment operator called");
	this->_client_ptr = other._client_ptr;
	this->_input_pipe[0] = other._input_pipe[0];
	this->_input_pipe[1] = other._input_pipe[1];
	this->_output_pipe[0] = other._output_pipe[0];
	this->_output_pipe[1] = other._output_pipe[1];
	this->_start_time = other._start_time;
	this->_exec_path = other._exec_path;
	this->_envp = other._envp;
	return *this;
}

CGIControler::~CGIControler(void) {
	// log_info("CGIControler destructor called");
}

exit_status CGIControler::initiate_cgi(const ClientConnection *client, const cgiConfig *cgi) {
	this->_client_ptr = client;
	build_envp(cgi);
	if (_client_ptr->get_method() == "POST") {
		if (pipe(_input_pipe) == -1) {
			log_error("Failed to create input pipe for CGI");
			return PIPE_FAILURE;
		}
	}
	if (pipe(_output_pipe) == -1) {
		log_error("Failed to create output pipe for CGI");
		return PIPE_FAILURE;
	}
	return SUCCESS;
}

pid_t	CGIControler::fork_dup_op(void) {
	pid_t pid = fork();
	if (pid == 0) {
		if (_client_ptr->get_method() == "POST") {
			dup2(_input_pipe[0], STDIN_FILENO);
			close(_input_pipe[1]);
		}
		dup2(_output_pipe[1], STDOUT_FILENO);
		close(_output_pipe[0]);
	}
	else if (pid > 0) {
		if (_client_ptr->get_method() == "POST") {
			close(_input_pipe[0]);
		}
		close(_output_pipe[1]);
		this->_start_time = time(NULL);
	}
	return pid;
}

void	CGIControler::build_envp(const cgiConfig *cgi) {
	_envp.clear();
	_envp.reserve(14);
	if (_client_ptr == NULL)
		return;

	const std::string method = _client_ptr->get_method();
	const std::string uri = _client_ptr->get_uri();
	std::string::size_type qpos = uri.find('?');
	if (qpos == std::string::npos)
		_script_name = uri;
	else {
		_script_name = uri.substr(0, qpos);
		_query_string = uri.substr(qpos + 1);
	}

	const std::map<std::string, std::string> & headers = _client_ptr->get_headers();
	std::string host;
	std::string content_type;
	std::string content_length;

	std::map<std::string, std::string>::const_iterator it = headers.find("Host");
	if (it != headers.end())
		host = trim_copy(it->second);
	it = headers.find("Content-Type");
	if (it != headers.end())
		content_type = trim_copy(it->second);
	it = headers.find("Content-Length");
	if (it != headers.end())
		content_length = trim_copy(it->second);
	if (content_length.empty() && method == "POST") {
		std::stringstream ss;
		ss << _client_ptr->get_body().size();
		content_length = ss.str();
	}

	push_env(_envp, "GATEWAY_INTERFACE", "CGI/1.1");
	push_env(_envp, "SERVER_PROTOCOL", "HTTP/1.1");
	push_env(_envp, "REQUEST_METHOD", method);
	push_env(_envp, "SCRIPT_NAME", _script_name);
	push_env(_envp, "PATH_INFO", _script_name);
	push_env(_envp, "QUERY_STRING", _query_string);
	push_env(_envp, "REMOTE_ADDR", "127.0.0.1");
	push_env(_envp, "SERVER_SOFTWARE", "webserv/1.0");

	if (cgi != NULL && !cgi->script_path.empty())
		push_env(_envp, "SCRIPT_FILENAME", cgi->script_path);
	if (!host.empty())
		push_env(_envp, "HTTP_HOST", host);
	if (!content_type.empty())
		push_env(_envp, "CONTENT_TYPE", content_type);
	if (!content_length.empty())
		push_env(_envp, "CONTENT_LENGTH", content_length);
}

void	CGIControler::execute_cgi(void) const {
	// need to chdir to the file to execute 
	// create argv with executable + file name to execute + NULL
	// launch execve
	std::string dir_path = _client_ptr->get_uri();
	std::vector<char *> envp_cstr;
	envp_cstr.reserve(_envp.size() + 1);
	for (size_t i = 0; i < _envp.size(); ++i) {
		envp_cstr.push_back(const_cast<char *>(_envp[i].c_str()));
	}
	envp_cstr.push_back(NULL);

	execve(_exec_path.c_str(), NULL, envp_cstr.data());
}

int CGIControler::get_input_w_pipe(void) const {
	return this->_input_pipe[1];
}

int CGIControler::get_output_r_pipe(void) const {
	return this->_output_pipe[0];
}

time_t CGIControler::get_start_time(void) const {
	return this->_start_time;
}

const std::vector<std::string> &	CGIControler::get_envp(void) const {
	return this->_envp;
}
