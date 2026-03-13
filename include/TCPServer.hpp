#ifndef TCPSERVER_HPP
# define TCPSERVER_HPP

# include "config.hpp"
# include "cgiControler.hpp"
# include "ClientConnection.hpp"
# include "Poller.hpp"
# include "Socket.hpp"

class TCPServer {
	private:
		TCPServer(void);

	public:
		TCPServer(const std::set<serverConfig> &serversConfig);
		TCPServer(const TCPServer &other);
		TCPServer&	operator=(const TCPServer &other);
		~TCPServer(void);

		static void		signal_handler(int signum);

		int			init(void);
		int			wait(void);

	private:
		void			close_fd(std::string msg, int fd);
		int				is_a_socket(int fd);
		int				is_a_client(int fd);

		int						add_new_client();
		const serverConfig *	find_server_config(int fd);
		exit_status				handle_client_event(int fd);

		bool			check_for_cgi(void);
		exit_status		activate_cgi(void);
		exit_status		handle_cgi_event(int fd);
		exit_status		fork_and_exec_cgi(CGIControler &cgi);

	private:
		std::set<serverConfig>			_servers;
		Poller							_poller;
		std::vector<Socket>				_sockets;
		std::vector<ClientConnection>	_clients;
		// std::vector<CGIControler>		_cgis;
		// LATER
		static bool						_close_server;

	private:
		// to reduce looping on struct
		Socket				*_socket_ptr;
		ClientConnection	*_client_ptr;
};

#endif
