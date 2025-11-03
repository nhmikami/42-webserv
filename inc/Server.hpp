#ifndef SERVER_HPP
#define SERVER_HPP

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>

class Server {
	private:
		std::string _file_path;
		std::string root;


	public:
		Server(void);
		Server(const Server &other);
		Server(std::string file_path);
		~Server(void);

		Server &operator=(const Server &other);
};

#endif