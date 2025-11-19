#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include "Logger.hpp"
#include "ParseUtils.hpp"

#include <poll.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <stdio.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>

class Server {
	private:
		int							_server_fd;
		struct sockaddr_in			_address;
		socklen_t					_addlen;
		std::string					_host;
		int							_port;
		std::vector<struct pollfd>	_fds;
		std::vector<Client*>		_clients;
		Logger						_logger;
		ParseUtils					_utils;

		bool	startServer();
		bool	bindServer();
		bool	startListen();
		bool	addToFDs(int fd);
		void	acceptClient();
		bool	handleClient(int i);
		void	closeClient(int i, int j, Client *client);

	public:
		Server(void); //private
		Server(const Server &other); //private
		Server(std::string host, int port);
		~Server(void);

		Server &operator=(const Server &other);

		void run();
};

#endif

/*
struct Server {
std::string host;
    int port;
    std::vector<std::string> server_names;
    std::string root;
    std::vector<std::string> index;
    std::map<int,std::string> error_pages;
    std::vector<Location> locations;
    bool is_default = false;
};
*/