#ifndef CGI_CONTROLER_HPP
# define CGI_CONTROLER_HPP

# include <vector>

# include "config.hpp"
# include "main.hpp"

class	ClientConnection;

class CGIControler {
	public:
		CGIControler(const int client_fd);
		CGIControler(const CGIControler & other);
		CGIControler & operator=(const CGIControler & other);
		~CGIControler(void);

		exit_status		initiate_cgi(const ClientConnection & client);
		pid_t			fork_dup_op(const ClientConnection & client);;
		void			build_envp(const ClientConnection & client, const cgiConfig& cgi);
		exit_status		execute_cgi() const;
		bool			feed_cgi_process(void);
		void			cgi_received_data(void);

		void			failed_execve_close(void) const;

		int									get_client_fd(void) const;
		int									get_input_w_pipe(void) const;
		int									get_output_r_pipe(void) const;
		const std::vector<std::string> &	get_envp(void) const;
		time_t								get_start_time(void) const;
		pid_t								get_child_pid(void) const;
		std::string							get_exec_path(void) const;

		std::string							get_received_data(void) const;

	private:
		CGIControler(void);

		int		_client_fd;
		int		_input_pipe[2];
		int		_output_pipe[2];
		time_t	_start_time;
		pid_t	_child_pid;

		std::string					_exec_path;
		std::string					_dir_path;
		std::string					_script_name;
		std::string					_query_string;

		std::vector<std::string>	_envp;

	private:
		std::string	_sent_data;
		std::string	_received_data;
};

#endif
