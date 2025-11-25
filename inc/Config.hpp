#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

struct Config {
	std::string	host;
	int			port;
	std::string	root;

	Config() {
		host = "127.0.0.1";
		port = 8080;
		root = "/data/www/";
	}
};

#endif