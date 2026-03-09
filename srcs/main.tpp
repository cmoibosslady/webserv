#include "main.hpp"

template<typename T> void log_connect(const T & input) {
	std::time_t now = std::time(NULL);
	char buffer[80];

	std::strftime(buffer, sizeof(buffer),
				  "%Y-%m-%d %H:%M:%S ",
				  std::localtime(&now));
	std::cout << GREEN << " [LOG] " << buffer << input << RESET << std::endl;
}

template<typename T> void log_info(const T & input)  {
    std::time_t now = std::time(NULL);
    char buffer[80];

    std::strftime(buffer, sizeof(buffer),
                  "%Y-%m-%d %H:%M:%S ",
                  std::localtime(&now));
	std::cout << BLUE << " [INFO] " << buffer  << input << RESET << std::endl;
}

template<typename T> void log_warning(const std::string & str, const T & input)  {
	std::time_t now = std::time(NULL);
	char buffer[80];

	std::strftime(buffer, sizeof(buffer),
				  "%Y-%m-%d %H:%M:%S ",
				  std::localtime(&now));
	std::cout << PURPLE << " [WARNING] " << buffer << str << input << RESET << std::endl;
}

template<typename T> void	log_error(const T & message) {
	std::cerr << RED << "[ERROR] " << message << RESET << std::endl;
}

template<typename T> void	log_debug(const std::string & message, const T & variable) {
	std::cout << CYAN << "[DEBUG] " << message << ": " << variable << RESET << std::endl;
}

