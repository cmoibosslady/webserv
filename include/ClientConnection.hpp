#ifndef CLIENTCONNECTION_HPP
# define CLIENTCONNECTION_HPP

// Cette classe représente un client.
# include <ctime>
# include <string>

# include "config.hpp"
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
		void	setServerConfig(const serverConfig * config);

		bool	needs_cgi(void) const;

		client_status	processTransmit(void);
		client_status	prepareResponse(void);
		client_status	send_response(void);

	private:
		int						_fd;
		std::string				_buffer;
		client_status			_status;
		time_t					_lastActivity;

		const serverConfig		* _server;
		const locationConfig	* _location;
};

#endif
