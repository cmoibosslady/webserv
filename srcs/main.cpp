#include "main.hpp"
#include "tokeniser.hpp"
#include "config.hpp"

int	main(int argc, char **argv)
{
	if (argc != 2) {
		log_error<std::string>("Usage: ./webserv <config_file>");
		return 1;
	}
	config config_setup(argv[1]);
}
