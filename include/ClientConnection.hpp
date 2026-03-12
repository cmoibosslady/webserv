#ifndef CLIENTCONNECTION_HPP
# define CLIENTCONNECTION_HPP

// Cette classe représente un client.
# include <ctime>
# include <string>

# include "main.hpp"
# include "Parser.hpp"
# include "Response.hpp"

class ClientConnection : public Parser, public Response 
{
	private:
		ClientConnection(void);

	public:
		ClientConnection(int fd);
		ClientConnection(const ClientConnection& other);
		ClientConnection& operator=(const ClientConnection& other);
		~ClientConnection(void);

		bool	operator==(const ClientConnection& other) const;

		int 	getFd() const;
		int		closeConnection(void);
		int		getStatus() const;

		void	updateLastActivity(void);
		bool	needs_cgi(void) const;

		client_status	processTransmit(void);
		client_status	sendResponse(void);

	private:
		int				_fd;
		std::string		_buffer;
		client_status	_status;
		time_t			_lastActivity;
};

#endif
