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

template<typename T> void log_connect(const T & input);
template<typename T> void log_info(const T & input);
template<typename T> void log_warning(const std::string & str, const T & input);
template<typename T> void	log_error(const T & message);
template<typename T> void	log_debug(const std::string & message, const T & variable);

#endif
