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

		int	init(void);

	private:
		std::set<serverConfig>			_servers;
		Poller							_poller;
		std::vector<Socket>				_sockets;
		std::vector<ClientConnection>	_clients;
};

#endif
