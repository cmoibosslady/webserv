#ifndef REQUEST_PROCESSOR_HPP
# define REQUEST_PROCESSOR_HPP

class ClientConnection;

enum processing_result {
	PROCESS_SUCCESS,
	NOT_FOUND,
	NOT_ALLOWED,
	INTERNAL_ERROR,
};

class RequestProcessor {

	public:
		RequestProcessor(void);
		RequestProcessor(const RequestProcessor & other);
		RequestProcessor&	operator=(const RequestProcessor & other);
		~RequestProcessor(void);

		processing_result	process_request(ClientConnection &client);
	
	private:
		processing_result	_res;
};

#endif
