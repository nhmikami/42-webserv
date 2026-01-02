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

#define SESSION_MAX_KEY_SIZE	64
#define SESSION_MAX_VALUE_SIZE	1024
#define SESSION_MAX_ENTRIES		50
#define SESSION_TIMEOUT			300

class Server {
	private:
		std::vector<ServerConfig>		_configs;
		std::vector<Client*>			_clients;
		std::map<int, ServerConfig*>	_fd_to_config;
		std::map<int, ServerConfig*>	_client_to_config;
		std::vector<struct pollfd>		_fds;

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
		Client*			findClient(size_t *j, int client_fd);
		ServerConfig*	findServerConfig(int client_fd);
		bool			handleClient(int i);
		void			unhandleClient(int i);
		bool			resetClient(size_t i, size_t j, Client* cli);
		void			closeClient(int j, Client *cli);
		void			enablePollOut(int client_fd);
		void			disablePollOut(size_t client_fd);

		bool			_isMethodAllowed(const std::string& method, const LocationConfig* loc);
		bool 			_processRequest(Request& req, Client* cli, ServerConfig* cfg, const LocationConfig* loc);
		bool			_processRedirect(int code, Client* cli, const LocationConfig* loc);
		bool			_processCgi(Client* cli, AMethod* method);
		bool			_handleCgiEvent(size_t i, short revents);
		void			_registerCgiHandler(CgiHandler *cgi, Client *cli);
		void			_finalizeCgiResponse(size_t i, int cgi_fd);
		bool			_completeResponse(HttpStatus status, Client* cli, ServerConfig* cfg, const LocationConfig* loc, AMethod* method);

		Session*		_handleSession(const Request& request);
		void			_processSessionData(Response& response, Session* session);

	public:
		Server(void); //private del?
		Server(std::string host, int port); //del
		Server(std::vector<ServerConfig> configs);
		~Server(void);

		void run(void);
};

void printRequest(ParseHttp parser);

#endif