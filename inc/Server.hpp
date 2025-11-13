#ifndef SERVER_HPP
#define SERVER_HPP

#include <Client.hpp>

#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <stdio.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>

class Server {
	private:
		int					_server_fd;
		struct sockaddr_in	_address;
		socklen_t			_addlen;
		std::string			_host;
		int					_port;


		bool	startServer();
		bool	bindServer();
		bool	startListen();
	public:
		Server(void); //private
		Server(const Server &other); //private
		Server(std::string host, int port);
		~Server(void);

		Server &operator=(const Server &other);
<<<<<<< HEAD
=======

		// bool	acceptClient(int &client_fd);
		// void	handleClient(int client_fd);
		// void	closeServer();
		void run();
>>>>>>> 3447155603e189226cb7605a539b7c10e869cb27
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