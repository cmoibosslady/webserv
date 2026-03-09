#ifndef CLIENTCONNECTION_HPP
# define CLIENTCONNECTION_HPP

// Cette classe représente un client.
# include <ctime>
# include <string>

# include "main.hpp"

class ClientConnection
{
	private:
		ClientConnection(void);

	public:
		ClientConnection(int fd);
		ClientConnection(const ClientConnection& other);
		ClientConnection& operator=(const ClientConnection& other);
		~ClientConnection(void);

		int 	getFd() const;
		int		closeConnection(void);
		int		getStatus() const;

	private:
		int				_fd;
		std::string		_buffer;
		client_status	_status;
		time_t			_lastActivity;
};

#endif
