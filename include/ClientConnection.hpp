#ifndef CLIENTCONNECTION_HPP
# define CLIENTCONNECTION_HPP

// Cette classe représente un client.
# include <ctime>
# include <string>

# include "config.hpp"
# include "main.hpp"
# include "Parser.hpp"
# include "Response.hpp"

enum request_type {
	REDIRECT,
	CGI,
	STATIC_FILE,
	UPLOAD,
	DELETE,
	AUTO_INDEX,
	UNKNOWN
};

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

		int 					getFd() const;
		const std::string &		get_method(void) const;
		const std::string &		get_uri(void) const;
		const std::map<std::string, std::string> &	get_headers(void) const;
		const std::string &		get_body(void) const;
		int						closeConnection(void);
		int						getStatus() const;

		void		updateLastActivity(void);
		void		setServerConfig(const serverConfig * config);
		void		setLocationConfig();
		std::string	getLocationRoot() const;
		void		setBuffer(const std::string cgi_answer);

		bool			needs_redirect(void);
		request_type	find_type_request(void);

		const cgiConfig *	needs_cgi(void) const;
		exit_status			launch_execve(void);

		bool	needs_static_response(void);
		int		get_on_file();

		client_status		processTransmit(void);
		client_status		prepareResponse(int http_status_code = 0, std::string content = ""); // to delete afterwards
		client_status		send_response(void);

	private:
		int						_fd;
		std::string				_buffer;
		client_status			_status;
		time_t					_lastActivity;

		const serverConfig		* _server;
		const locationConfig	* _location;
		const rewriteConfig		*_rewrite;
};

#endif
