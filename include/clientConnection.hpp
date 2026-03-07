#ifndef CLIENTCONNECTION_HPP
# define CLIENTCONNECTION_HPP

// Cette classe représente un client.
# include <ctime>
# include <string>

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
		int		getStatus() const;

	private:
		int _fd;
		std::string _buffer;
		int	_status; // READING_HEADERS, READING_BODY, SENDING, CLOSING
		time_t _lastActivity;
};

#endif
