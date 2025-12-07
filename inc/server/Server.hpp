#ifndef SERVER_HPP
#define SERVER_HPP

#include "server/Client.hpp"
#include "utils/Logger.hpp"
#include "utils/ParseUtils.hpp"
#include "config/ServerConfig.hpp"
#include "parse/ParseHttp.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "MethodGET.hpp"
#include "MethodPOST.hpp"
#include "MethodDELETE.hpp"

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
		std::vector<ServerConfig>		_configs;
		std::map<int, ServerConfig*>	_fd_to_config;
		std::map<int, ServerConfig*>	_client_to_config;
		std::vector<struct pollfd>		_fds;
		std::vector<Client*>			_clients;

		Server(const Server &other); //del?
		
		Server &operator=(const Server &other);

		bool			startServer();
		bool			bindServer(int server_fd, struct sockaddr_in address, int port);
		bool			startListen(int server_fd, std::string host, int port);
		bool			addToFDs(int server_fd);
		void			acceptClient(int server_fd, ServerConfig *config);
		Client			*findClient(size_t *j, int client_fd);
		ServerConfig	*findServerConfig(int client_fd);
		bool			handleClient(int i);
		void			unhandleClient(int i);
		void			closeClient(int i, int j, Client *client);
		bool			isMethodAllowed(const std::string& method, const LocationConfig* location);

	public:
		Server(void); //private del?
		Server(std::string host, int port); //del
		Server(std::vector<ServerConfig> configs);
		~Server(void);

		void run();
};

void printRequest(ParseHttp parser);

#endif