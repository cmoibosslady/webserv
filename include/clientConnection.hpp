#ifndef CLIENTCONNECTION_HPP
# define CLIENTCONNECTION_HPP

// Cette classe represent un client.
# include <ctime>
# include <string>

class ClientConnection
{
	public:
		ClientConnection();
		ClientConnection(const ClientConnection& other);
		ClientConnection& operator=(const ClientConnection& other);
		~ClientConnection();

	private:
		int _fd;
		std::string _buffer;
		bool	_status; // READING_HEADERS, READING_BODY, SENDING, CLOSING
		time_t _lastActivity;
};

#endif
