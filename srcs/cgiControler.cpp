#include <cctype>
#include <csignal>
#include <sstream>
#include <unistd.h>
#include "cgiControler.hpp"
#include "ClientConnection.hpp"
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

CGIControler::CGIControler(void): _client_fd(-1) {
	log_info("CGIControler default private constructor called");
}

CGIControler::CGIControler(const int client_fd): _client_fd(client_fd) {
	// log_info("CGIControler constructor called");
	_input_pipe[0] = -1;
	_input_pipe[1] = -1;
	_output_pipe[0] = -1;
	_output_pipe[1] = -1;
}

CGIControler::CGIControler(const CGIControler & other) {
	// log_info("CGIControler copy constructor called");
	*this = other;
}

CGIControler & CGIControler::operator=(const CGIControler & other) {
	// log_info("CGIControler assignment operator called");
	this->_client_fd = other._client_fd;
	this->_input_pipe[0] = other._input_pipe[0];
	this->_input_pipe[1] = other._input_pipe[1];
	this->_output_pipe[0] = other._output_pipe[0];
	this->_output_pipe[1] = other._output_pipe[1];
	this->_start_time = other._start_time;
	this->_child_pid = other._child_pid;

	this->_exec_path = other._exec_path;
	this->_dir_path = other._dir_path;
	this->_script_name = other._script_name;
	this->_query_string = other._query_string;
	this->_envp = other._envp;
	
	this->_sent_data = other._sent_data;
	this->_received_data = other._received_data;
	return *this;
}

CGIControler::~CGIControler(void) {
	// log_info("CGIControler destructor called");
}

bool	CGIControler::operator==(const CGIControler & other) const {
	return this->_child_pid == other._child_pid;
}

exit_status CGIControler::initiate_cgi(const ClientConnection & client) {
	if (client.get_method() == "POST") {
		if (pipe(_input_pipe) == -1) {
			log_error("Failed to create input pipe for CGI");
			return PIPE_FAILURE;
		}
		_sent_data = client.get_body();
	}
	if (pipe(_output_pipe) == -1) {
		log_error("Failed to create output pipe for CGI");
		return PIPE_FAILURE;
	}
	return SUCCESS;
}

pid_t	CGIControler::fork_dup_op(const ClientConnection & client) {
	_child_pid = fork();
	if (_child_pid == 0) {
		signal(SIGINT, SIG_DFL);
		if (client.get_method() == "POST") {
			close(_input_pipe[1]);
			dup2(_input_pipe[0], STDIN_FILENO);
			close(_input_pipe[0]);
		}
		close(_output_pipe[0]);
		dup2(_output_pipe[1], STDOUT_FILENO);
		close(_output_pipe[1]);
	}
	else if (_child_pid > 0) {
		if (client.get_method() == "POST") {
			close(_input_pipe[0]);
		}
		close(_output_pipe[1]);
		this->_start_time = time(NULL);
	}
	return _child_pid;
}

void	CGIControler::build_envp(const ClientConnection & client, const cgiConfig& cgi) {
	_exec_path = cgi.script_path;

	_envp.clear();
	_envp.reserve(14);

	const std::string method = client.get_method();
	const std::string uri = client.get_uri();
	const std::string root = client.getLocationRoot();
	// log_error("Building CGI environment for URI: " + uri);
	std::string::size_type qpos = uri.find('?');
	if (qpos == std::string::npos)
		_script_name = uri.substr(1);
	else {
		_script_name = uri.substr(1, qpos - 1);
		_query_string = uri.substr(qpos + 1);
	}
	if (_script_name.find_last_of('/') != std::string::npos)
		_dir_path = root + _script_name.substr(0, _script_name.find_last_of('/'));
	else
		_dir_path = root;
	if (_dir_path.empty())
		_dir_path = ".";

	const std::map<std::string, std::string> & headers = client.get_headers();
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
		ss << client.get_body().size();
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

	if (!cgi.script_path.empty())
		push_env(_envp, "SCRIPT_FILENAME", cgi.script_path);
	if (!host.empty())
		push_env(_envp, "HTTP_HOST", host);
	if (!content_type.empty())
		push_env(_envp, "CONTENT_TYPE", content_type);
	if (!content_length.empty())
		push_env(_envp, "CONTENT_LENGTH", content_length);
}

exit_status	CGIControler::execute_cgi(void) const {
	if (chdir(_dir_path.c_str()) == -1) {
		log_error("Failed to change directory to " + _dir_path);
		failed_execve_close();
		return CHDIR_FAILURE;
	}

	std::vector<char *> envp_cstr;
	envp_cstr.reserve(_envp.size() + 1);
	for (size_t i = 0; i < _envp.size(); ++i) {
		envp_cstr.push_back(const_cast<char *>(_envp[i].c_str())); // seems so dirty
	}
	envp_cstr.push_back(NULL);

	std::vector<char *> argv;
	argv.push_back(const_cast<char *>(_exec_path.c_str())); // hating this
	argv.push_back(const_cast<char *>(_script_name.c_str()));
	argv.push_back(NULL);

	execve(_exec_path.c_str(), argv.data(), envp_cstr.data());
	failed_execve_close();
	return EXECVE_FAILURE;
}

bool	CGIControler::feed_cgi_process(void) {
	if (_sent_data.empty())
		return true;

	ssize_t bytes_written = write(_input_pipe[1], _sent_data.c_str(), _sent_data.size());
	if (bytes_written < 0) {
		log_error("Failed to write to CGI process");
		return false;
	}
	_sent_data.erase(0, bytes_written);
	return _sent_data.empty();
}

void	CGIControler::cgi_received_data(void) {
	char buffer[BUFFER_SIZE];
	ssize_t bytes_read = read(_output_pipe[0], buffer, BUFFER_SIZE - 1);
	if (bytes_read < 0) {
		log_error("Failed to read from CGI process");
		return ;
	}
	buffer[bytes_read] = '\0';
	_received_data += buffer;
}

void	CGIControler::failed_execve_close(void) const {
	if (_input_pipe[0] != -1)
		close(STDIN_FILENO);
	close(STDOUT_FILENO);
}

int	CGIControler::get_client_fd(void) const {
	return this->_client_fd;
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

pid_t	CGIControler::get_child_pid(void) const {
	return this->_child_pid;
}

std::string	CGIControler::get_exec_path(void) const {
	return this->_exec_path;
}

std::string	CGIControler::get_received_data(void) const {
	return this->_received_data;
}
