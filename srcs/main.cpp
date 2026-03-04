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
	if (config_setup.parseConfFile() == -1) {
		log_error<std::string>("Failed to open config file: " + std::string(argv[1]));
		return 1;
	}
}
