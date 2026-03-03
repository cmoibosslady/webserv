#ifndef MAIN_HPP
# define MAIN_HPP

# include <ctime>
# include <iostream>
# include <string>

# define RED "\033[31m"
# define GREEN "\033[32m"
# define BLUE "\33[33m"
# define PURPLE "\033[35m"
# define CYAN "\033[36m"
# define RESET "\033[0m"

# define BUFFER_SIZE 65536

template<typename T> void log_info(const T & input)  {
    std::time_t now = std::time(NULL);
    char buffer[80];

    std::strftime(buffer, sizeof(buffer),
                  "%Y-%m-%d %H:%M:%S ",
                  std::localtime(&now));
	std::cout << BLUE << " [INFO] " << buffer  << input << RESET << std::endl;
}

template<typename T> void log_connect(const T & input) {
	std::time_t now = std::time(NULL);
	char buffer[80];

	std::strftime(buffer, sizeof(buffer),
				  "%Y-%m-%d %H:%M:%S ",
				  std::localtime(&now));
	std::cout << GREEN << " [LOG] " << buffer << input << RESET << std::endl;
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
	std::cerr << RED << "[ERROR] " << message << std::endl;
}

template<typename T> void	log_debug(const std::string & message, const T & variable) {
	std::cout << CYAN << "[DEBUG] " << message << ": " << variable << RESET << std::endl;
}

#endif
