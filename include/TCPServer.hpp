#ifndef TCPSERVER_HPP
# define TCPSERVER_HPP

# include "config.hpp"
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

		int			init(void);
		int			wait(void);

	private:
		void		close_fd(std::string msg, int fd);
		int			is_a_socket(int fd);
		int			is_a_client(int fd);

		int			add_new_client();
		exit_status	handle_client_event(int fd);

	private:
		std::set<serverConfig>			_servers;
		Poller							_poller;
		std::vector<Socket>				_sockets;
		std::vector<ClientConnection>	_clients;

	private:
		// to reduce looping on struct
		const Socket			*_socket_ptr;
		const ClientConnection	*_client_ptr;
};

#endif
