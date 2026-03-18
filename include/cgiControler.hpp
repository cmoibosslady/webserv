#ifndef CGI_CONTROLER_HPP
# define CGI_CONTROLER_HPP

# include <vector>

# include "ClientConnection.hpp"
# include "main.hpp"

class CGIControler {
	public:
		CGIControler(void);
		CGIControler(const CGIControler & other);
		CGIControler & operator=(const CGIControler & other);
		~CGIControler(void);

		exit_status	initiate_cgi(const ClientConnection *client, const cgiConfig *cgi);
		pid_t		fork_dup_op(void);
		void		build_envp(const cgiConfig *cgi);
		void		execute_cgi() const;

		int									get_input_w_pipe(void) const;
		int									get_output_r_pipe(void) const;
		const std::vector<std::string> &	get_envp(void) const;
		time_t								get_start_time(void) const;

	private:
		const ClientConnection	*_client_ptr;
		int		_input_pipe[2];
		int		_output_pipe[2];
		time_t	_start_time;

		std::string					_exec_path;
		std::string					_script_name;
		std::string					_query_string;
		std::vector<std::string>	_envp;
};

#endif
