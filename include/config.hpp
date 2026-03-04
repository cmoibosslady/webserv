#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <map>
# include <set>
# include <string>

# include "main.hpp"

struct rewriteConfig {
	std::string pattern;
	std::string replacement;

	bool operator<(const rewriteConfig& other) const;
};

struct cgiConfig {
	std::string extension;
	std::string executable_path;

	std::set<std::string> allowed_methods; // by default, all methods are allowed
	bool operator<(const cgiConfig& other) const;
};

struct locationConfig {
	std::string path;
	std::set<std::string> allowed_methods;
	std::string root;

	bool 	autoindex;
	bool 	upload_auth;
	std::string upload_path;

	std::set<std::string> index_files;

	std::set<rewriteConfig> rewrites;
	std::set<cgiConfig> cgi_configs;

	bool operator<(const locationConfig& other) const;
};

struct serverConfig {
	int port;

	std::map<int, std::string> error_pages;
	size_t	client_max_body_size;

	std::set<locationConfig> locations;

	bool operator<(const serverConfig& other) const;
};

// config class from the configuration file

class config {
	private:
		config(void);

	public:
		config(const config & other);
		config&	operator=(const config & other);
		~config(void);
		
		config(const std::string &config_file);

		int 	parseConfFile(std::set<serverConfig> &) const;

	private:
		int		checkFile(const char *__restrict__ file_path) const;

	private:
		std::string _config_file;
		char 		_buffer[BUFFER_SIZE];


};

# endif
