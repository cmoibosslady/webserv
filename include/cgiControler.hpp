#ifndef CGI_CONTROLER_HPP
# define CGI_CONTROLER_HPP

# include "ClientConnection.hpp"

class CGIControler {
	public:
		CGIControler(void);
		CGIControler(const CGIControler & other);
		CGIControler & operator=(const CGIControler & other);
		~CGIControler(void);

		void	initiate_cgi(const ClientConnection *client);

	private:
		const ClientConnection	*_client_ptr;
		int		_input_pipe[2];
		int		_output_pipe[2];
		time_t	_start_time;
};

#endif
