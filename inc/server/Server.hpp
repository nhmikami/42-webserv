#ifndef SERVER_HPP
#define SERVER_HPP

#include "config/ServerConfig.hpp"
#include "server/Client.hpp"
#include "server/CgiHandler.hpp"
#include "server/SessionManager.hpp"
#include "parse/ParseHttp.hpp"
#include "http/Request.hpp"
#include "http/Response.hpp"
#include "http/MethodGET.hpp"
#include "http/MethodPOST.hpp"
#include "http/MethodDELETE.hpp"
#include "utils/Logger.hpp"
#include "utils/ParseUtils.hpp"

#include <map>
#include <set>
#include <vector>
#include <string>
#include <cstring>
#include <fstream>
#include <iostream>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>

class Server {
	private:
		std::vector<ServerConfig>		_configs;
		std::map<int, ServerConfig*>	_fd_to_config;
		std::map<int, ServerConfig*>	_client_to_config;
		std::vector<struct pollfd>		_fds;
		std::vector<Client*>			_clients;

		std::map<int, CgiHandler*>		_cgiHandlers;
		std::map<int, Client*>			_cgiClient;
		SessionManager					_sessions;

		Server(const Server &other); //del?
		
		Server &operator=(const Server &other);

		bool			startServer(void);
		bool			bindServer(int server_fd, struct sockaddr_in address, int port);
		bool			startListen(int server_fd, std::string host, int port);
		bool			addToFDs(int server_fd);
		void			acceptClient(int server_fd, ServerConfig *config);
		Client			*findClient(size_t *j, int client_fd);
		ServerConfig	*findServerConfig(int client_fd);
		bool			handleClient(int i);
		void			unhandleClient(int i);
		void			closeClient(int j, Client *client);

		bool	_isMethodAllowed(const std::string& method, const LocationConfig* location);
		bool	_parseRequest(const std::string& raw_request, Request& request, ServerConfig* config, Client* client, size_t j);
		bool 	_processRequest(Request& request, ServerConfig* config, const LocationConfig* location, Client* client, size_t j);
		bool	_processRedirect(int code, ServerConfig* config, const LocationConfig* location, Client* client, size_t j);
		bool	_processError(HttpStatus status, ServerConfig* config, const LocationConfig* location, Client* client, size_t j);
		bool	_processCgi(AMethod* method, Client* client, int client_fd);
		bool	_sendResponse(AMethod* method, HttpStatus status, Client* client);

		bool	_handleCgiEvent(size_t i);
		void	_registerCgiHandler(int client_fd, CgiHandler *cgi, Client *client);
		void	_finalizeCgiResponse(size_t index, int cgi_fd);
		void	_setCookies(Response& response, Session* session);
		Session*	_handleSession(const Request& request);

	public:
		Server(void); //private del?
		Server(std::string host, int port); //del
		Server(std::vector<ServerConfig> configs);
		~Server(void);

		void run(void);
};

void printRequest(ParseHttp parser);

#endif