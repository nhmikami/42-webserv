#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include "Logger.hpp"
#include "ParseUtils.hpp"
#include "ServerConfig.hpp"

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
		int							_server_fd; //del
		struct sockaddr_in			_address; //del
		socklen_t					_addlen; //del
		std::string					_host; //del
		int							_port; //del
		std::vector<ServerConfig>	_configs;
		std::map<int, ServerConfig*> _fd_to_config;
		std::map<int, ServerConfig*>	_client_to_config;
		std::vector<struct pollfd>	_fds;
		std::vector<Client*>		_clients;
		Logger						_logger; //del (acessar log sem inserir na classe - depois)

		Server(const Server &other); //del?
		
		Server &operator=(const Server &other);

		bool	startServer();
		bool	bindServer(int server_fd, struct sockaddr_in address, int port);
		bool	startListen(int server_fd, std::string host, int port);
		bool	addToFDs(int server_fd);
		void	acceptClient(int server_fd, ServerConfig *config);
		Client	*findClient(int i);
		bool	handleClient(int i);
		void	unhandleClient(int i);
		void	closeClient(int i, int j, Client *client);

	public:
		Server(void); //private del?
		Server(std::string host, int port); //del
		Server(std::vector<ServerConfig> configs);
		~Server(void);

		void run();
};

#endif
