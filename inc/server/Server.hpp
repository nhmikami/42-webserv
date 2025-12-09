#ifndef SERVER_HPP
#define SERVER_HPP

#include "server/Client.hpp"
#include "utils/Logger.hpp"
#include "utils/ParseUtils.hpp"
#include "config/ServerConfig.hpp"
#include "parse/ParseHttp.hpp"
#include "http/Request.hpp"
#include "http/Response.hpp"
#include "http/MethodGET.hpp"
#include "http/MethodPOST.hpp"
#include "http/MethodDELETE.hpp"

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

		std::map<int, CgiHandler*>		_cgiHandlers;
		std::map<int, Client*>			_cgiClient;

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

		bool	_isMethodAllowed(const std::string& method, const LocationConfig* location);
		bool	_parseRequest(const std::string& raw_request, Request& request, ServerConfig* config, Client* client, int i, size_t j);
		bool 	_processRequest(Request& request, ServerConfig* config, const LocationConfig* location, Client* client, int i, size_t j);
		bool	_processError(HttpStatus status, ServerConfig* config, const LocationConfig* location, Client* client, int i, size_t j);
		bool	_processCgi(AMethod* method, Client* client, int client_fd);
		bool	_sendResponse(AMethod* method, HttpStatus status, Client* client);

		bool	_handleCgiEvent(size_t i);
		void	_registerCgiHandler(int client_fd, CgiHandler *cgi, Client *client);
		void	_finalizeCgiResponse(size_t index, int cgi_fd);

	public:
		Server(void); //private del?
		Server(std::string host, int port); //del
		Server(std::vector<ServerConfig> configs);
		~Server(void);

		void run();
};

void printRequest(ParseHttp parser);

#endif