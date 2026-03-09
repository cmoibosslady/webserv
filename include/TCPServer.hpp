#ifndef TCPSERVER_HPP
# define TCPSERVER_HPP

# include "Poller.hpp"

class TCPServer {
	public:
		TCPServer(void);
		TCPServer(const TCPServer &other);
		TCPServer&	operator=(const TCPServer &other);
		~TCPServer(void);

		int	init(int port);

	private:
		Poller	_poller;
};

#endif
