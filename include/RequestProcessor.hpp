#ifndef REQUEST_PROCESSOR_HPP
# define REQUEST_PROCESSOR_HPP

class ClientConnection;

enum processing_result {
	STATIC_FILE,
	POST_REQUEST,
	DELETE_REQUEST,
	CGI_REQUEST,
	REDIRECTION,
	NOT_ALLOWED,
};

class RequestProcessor {

	public:
		RequestProcessor(void);
		RequestProcessor(const RequestProcessor & other);
		RequestProcessor&	operator=(const RequestProcessor & other);
		~RequestProcessor(void);

		processing_result	process_request(ClientConnection &client);
};

#endif
