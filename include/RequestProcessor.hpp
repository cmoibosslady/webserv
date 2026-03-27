#ifndef REQUEST_PROCESSOR_HPP
# define REQUEST_PROCESSOR_HPP

class ClientConnection;

enum processing_result {
	STATIC_FILE,
	POST_REQUEST,
	CGI_REQUEST,
	REDIRECTION,
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
	
	// Quels sont les types de demande que je peux avoir?
	// 		- GET tout simple
	// 		- HEAD tout simple
	// 		- POST tout simple
	// 		- DELETE tout simple
	// 		- GET en cgi
	// 		- POST en cgi
	// Premiere grille d'identification : 
	// 		- est-ce une redirect ? -> status 301/307
	// 		- le fichier existe-t-il ? -> error 404
	// 		- la methode est-elle autorisee ? -> error 403
	// 		- CGI ou non
	// Si oui -> faire le cgi
	// Si non passer a la deuxieme verification
	// 		- c'est quoi comme POST ?
	// Comment je peux les identifier 

	private:
		processing_result	_res;
};

#endif
