#include "server/Server.hpp"

Server::Server(std::vector<ServerConfig> configs) : _configs(configs) {
	if (!startServer())
		Logger::log(Logger::ERROR, "Failed to start server.");
}

Server::~Server(void) {
	for (size_t i = 0; i < _clients.size(); i++) {
		delete _clients[i];
	}
	_clients.clear();

	for (size_t i = 0; i < _fds.size(); i++) {
		if (_fds[i].fd >= 0)
			close(_fds[i].fd);
	}
	_fds.clear();

	for (std::map<int, CgiHandler*>::iterator it = _cgiHandlers.begin(); it != _cgiHandlers.end(); ++it) {
		delete it->second;
	}
	_cgiHandlers.clear();
	_cgiClient.clear();
	_fd_to_config.clear();
	_client_to_config.clear();
};

bool	Server::startServer(void) {
	for (size_t i = 0; i < _configs.size(); i++) {
		int server_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (server_fd < 0){
			Logger::log(Logger::ERROR, "Socket failed.");
			return false;
		}
		
		int opt = 1;
		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
			Logger::log(Logger::ERROR, "Failed to set SO_REUSEADDR.");
			close(server_fd);
			return false;
		}

		if (fcntl(server_fd, F_SETFL, O_NONBLOCK) < 0) {
			Logger::log(Logger::ERROR, "Failed to set O_NONBLOCK.");
			close(server_fd);
			return false;
		}
		
		std::string host = _configs[i].getHost();
		int port = _configs[i].getPort();
		struct sockaddr_in address;
		std::memset(&address, 0, sizeof(address));
	
		address.sin_family = AF_INET;
		if (inet_pton(AF_INET, host.c_str(), &address.sin_addr) != 1)
			address.sin_addr.s_addr = htonl(INADDR_ANY);
		address.sin_port = htons(port);
	
		if (!(bindServer(server_fd, address, port) && startListen(server_fd, host, port) && addToFDs(server_fd))) {
			close(server_fd);
			return false;
		}
		_fd_to_config[server_fd] = &_configs[i];
	}
	return true;
};

bool	Server::bindServer(int server_fd, struct sockaddr_in address, int port) {
	if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
		Logger::log(Logger::ERROR, "Failed to bind server port: " + ParseUtils::itoa(port));
		return false;
	}
	return true;
};

bool	Server::startListen(int server_fd, std::string host, int port) {
	if (listen(server_fd, 128) < 0) {
		Logger::log(Logger::ERROR, "Failed to listen.");
		return false;
	}

	Logger::log(Logger::SERVER, "Listening on " + host + ":" + ParseUtils::itoa(port));
	return true;
};

bool	Server::addToFDs(int server_fd) {
	struct pollfd pollfd;
	pollfd.fd = server_fd;
	pollfd.events = POLLIN;
	pollfd.revents = 0;
	_fds.push_back(pollfd);

	return true;
};

void	Server::run(void) {
	while (true) {
		int res = poll(_fds.data(), _fds.size(), -1);
		if (res == -1) {
			Logger::log(Logger::ERROR, "Failed to poll.");
			continue ;
		} else if (res == 0) {
			continue ;
		}

		size_t i = 0;
		while (i < _fds.size()) {
			int fd = _fds[i].fd;
			if (_fds[i].revents == 0) {
				i++;
				continue ;
			}
			if (_fd_to_config.count(fd)) {
				if (_fds[i].revents & POLLIN) {
					acceptClient(fd, _fd_to_config[fd]);
				}
				i++;
			} else if (_cgiHandlers.count(fd)) {
				if (_handleCgiEvent(i))
					continue ;
				i++;
			} else {
				if (_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
					unhandleClient(i);
					continue ;
				} else if (_fds[i].revents & POLLIN) {
					if (handleClient(i))
						i++;
					else
						continue ;
				} else if (_fds[i].revents & POLLOUT) {
					i++;
				} else {
					i++;
				}
			}
		}
	}
}

void	Server::acceptClient(int server_fd, ServerConfig *config) {
	struct sockaddr_in address;
	socklen_t addrlen = sizeof(address);
	
	int client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen);
	if (client_fd < 0) {
		Logger::log(Logger::ERROR, "Failed to accept client.");
		return ;
	}

	if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0) {
		Logger::log(Logger::ERROR, "Failed to set O_NONBLOCK for client.");
		close(client_fd);
		return ;
	}

	addToFDs(client_fd);
	Client *client = new Client(client_fd);
	_clients.push_back(client);
	_client_to_config[client_fd] = config;

	Logger::log(Logger::SERVER, "New client accepted (fd=" + ParseUtils::itoa(client_fd) + ")");
};

Client	*Server::findClient(size_t *j, int client_fd) {
	while (*j < _clients.size()){
		if (_clients[*j]->getFd() == client_fd) {
			return _clients[*j];
		}
		(*j)++;
	}
	return NULL;
}

ServerConfig *Server::findServerConfig(int client_fd) {
	std::map<int, ServerConfig*>::iterator it = _client_to_config.find(client_fd);
	if (it == _client_to_config.end()) {
		Logger::log(Logger::ERROR, "Client config not found for fd=" + ParseUtils::itoa(client_fd));
		return NULL;
	}
	return it->second;
}

bool Server::handleClient(int i) {
	size_t	j = 0;
	int		client_fd = _fds[i].fd;
	Client*	client = findClient(&j, client_fd);
	if (!client)
		return true;

	ServerConfig* config = findServerConfig(client_fd);
	if (!config) {
		closeClient(j, client);
		return false;
	}
	
	std::pair<HttpStatus, ParseHttp>receive_parse_request = client->receive();
	HttpStatus status = receive_parse_request.first;
	ParseHttp& parser = receive_parse_request.second;

	// std::string raw_request = client->receive();
	if (status == SERVER_ERR) {
		Logger::log(Logger::SERVER, "Client disconnected (fd=" + ParseUtils::itoa(client_fd) + ")");
		closeClient(j, client);
		return false;
	}

    if (status >= BAD_REQUEST) {
        return _processError(status, config, NULL, client, j);
    }
	printRequest(parser);
	
    Request request = parser.buildRequest();
    
    client->setHttpVersion(request.getHttpVersion());
    client->setServerName(config->getServerName());


	// Logger::log(Logger::SERVER, "Received from fd=" + ParseUtils::itoa(client_fd) + ":\n" + raw_request);

	// Request	request;
	// if (!_parseRequest(raw_request, request, config, client, j)) {
	// 	std::cout << "Failed to parse request" << std::endl;
	// 	return false;
	// }

	const LocationConfig* location = config->findLocation(FileUtils::normalizePath(request.getPath()));
	return _processRequest(request, config, location, client, j);
}

// bool Server::_parseRequest(const std::string& raw_request, Request& request, ServerConfig* config, Client* client, size_t j) {
// 	ParseHttp	parser;
// 	std::string	req = raw_request;
// 	HttpStatus	status = parser.initParse(req);
// 	if (status >= BAD_REQUEST)
// 		return _processError(status, config, NULL, client, j);

// 	request = parser.buildRequest();
// 	client->setHttpVersion(request.getHttpVersion());
// 	client->setServerName(config->getServerName());
// 	// printRequest(parser); // for debugging
// 	return true;
// }

bool Server::_processRequest(Request& request, ServerConfig* config, const LocationConfig* location, Client* client, size_t j) {
	if (location && location->hasReturn())
		return _processRedirect(static_cast<HttpStatus>(location->getReturnCode()), config, location, client, j);

	if (!_isMethodAllowed(request.getMethodStr(), location))
		return _processError(NOT_ALLOWED, config, location, client, j);

	std::cout << "PROCESSANDO REQUEST" << std::endl;
	AMethod* method = NULL;
	if (request.getMethodStr() == "GET")
		method = new MethodGET(request, *config, location);
	else if (request.getMethodStr() == "POST")
		method = new MethodPOST(request, *config, location);
	else if (request.getMethodStr() == "DELETE")
		method = new MethodDELETE(request, *config, location);
	else {
		return _processError(NOT_IMPLEMENTED, config, location, client, j);
	}
	HttpStatus status = method->handleMethod();
	
	if (status == CGI_PENDING)
		return _processCgi(method, client, client->getFd());
	std::cout << "RESPONSE : " << status << std::endl;
	return _sendResponse(method, status, client);
}

bool Server::_isMethodAllowed(const std::string& method, const LocationConfig* location) {
	if (!location)
		return true;
	
	const std::set<std::string>& allowed_methods = location->getMethods();
	if (allowed_methods.empty())
		return true;
	if (allowed_methods.find(method) == allowed_methods.end())
		return false;

	return true;
}

bool Server::_processCgi(AMethod* method, Client* client, int client_fd) {
	CgiHandler* cgi = method->releaseCgiHandler();
	_registerCgiHandler(client_fd, cgi, client);

	Logger::log(Logger::SERVER, "CGI started for client fd=" + ParseUtils::itoa(client_fd));

	delete method;
	return true;
}

bool Server::_processRedirect(int code, ServerConfig* config, const LocationConfig* location, Client* client, size_t j) {
	Response res;
	res.setStatus(static_cast<HttpStatus>(code));
	std::string content = location->getReturnPath();
	if (code >= 300 && code < 400) {
		res.addHeader("Location", content);
		client->sendResponse(res.buildResponse(client->getServerName(), client->getHttpVersion()));
		return true;
	} else if ((code >= 200 && code < 300) || (code >= 400 && code < 600)) {
		res.setBody(content);
		res.addHeader("Content-Type", "text/plain");
		client->sendResponse(res.buildResponse(client->getServerName(), client->getHttpVersion()));
		return true;
	}
	return _processError(SERVER_ERR, config, location, client, j);
}

bool Server::_processError(HttpStatus status, ServerConfig* config, const LocationConfig* location, Client* client, size_t j) {
	std::string statusLine = ParseUtils::itoa(static_cast<int>(status)) + " " + Response(status).getStatusMessage();
	Logger::log(Logger::ERROR, "Error status: " + statusLine);
	
	Response res;
	if (config) {
		res.processError(status, *config, location);
	} else {
		res.setStatus(status);
		res.setBody("Fatal error");
		res.addHeader("Content-Type", "text/plain");
	}
	client->sendResponse(res.buildResponse(client->getServerName(), client->getHttpVersion()));
	closeClient(j, client);
	return false;
}

bool Server::_sendResponse(AMethod* method, HttpStatus status, Client* client) {
	Response res = method->getResponse();
	if (status >= 400)
		res.processError(status, method->getServerConfig(), method->getLocationConfig());
	else
		res.setStatus(status);

	if (status != NO_CONTENT && res.getHeader("Content-Length").empty()) {
		res.addHeader("Content-Length", ParseUtils::itoa(static_cast<int>(res.getBody().size())));
	}
	std::string response = res.buildResponse(client->getServerName(), client->getHttpVersion());
	client->sendResponse(response);
	
	delete method;
	return true;
}

void Server::unhandleClient(int i) {
	size_t	j = 0;
	int		client_fd = _fds[i].fd;
	Client	*client = findClient(&j, client_fd);

	if (client) {
		Logger::log(Logger::SERVER, "Connection error or hangup (fd=" + ParseUtils::itoa(client_fd) + ")");
		closeClient(j, client);
		return ;
	}
	close(_fds[i].fd);
	_fds.erase(_fds.begin() + i);
}

void	Server::closeClient(int j, Client *client) {
	if (!client)
		return ;
	int client_fd = client->getFd();

	std::vector<int> cgi_fds;
	for (std::map<int, Client*>::iterator it = _cgiClient.begin(); it != _cgiClient.end(); ++it) {
		if (it->second == client)
			cgi_fds.push_back(it->first);
	}

	for (size_t k = 0; k < cgi_fds.size(); ++k) {
		int cgi_fd = cgi_fds[k];
		close(cgi_fd);
		if (_cgiHandlers.count(cgi_fd)) {
			delete _cgiHandlers[cgi_fd];
			_cgiHandlers.erase(cgi_fd);
		}
		_cgiClient.erase(cgi_fd);
	}

	std::set<int> remove_set;
	for (size_t k = 0; k < cgi_fds.size(); ++k)
		remove_set.insert(cgi_fds[k]);
	remove_set.insert(client_fd);
	close(client_fd);

	for (size_t f = 0; f < _fds.size(); ) {
		if (remove_set.find(_fds[f].fd) != remove_set.end()) {
			_fds.erase(_fds.begin() + f);
		} else {
			++f;
		}
	}
	if (j >= 0 && (size_t)j < _clients.size()) {
		if (_clients[j] == client)
			_clients.erase(_clients.begin() + j);
		else {
			for (size_t k = 0; k < _clients.size(); ++k) {
				if (_clients[k] == client) {
					_clients.erase(_clients.begin() + k);
					break ;
				}
			}
		}
	}
	_client_to_config.erase(client_fd);
	delete client;
}

bool Server::_handleCgiEvent(size_t i) {
	int cgi_fd = _fds[i].fd;
	if (_cgiHandlers.count(cgi_fd) == 0)
		return false;
	CgiHandler *cgi = _cgiHandlers[cgi_fd];

	uint32_t events = 0;
	if (_fds[i].revents & POLLIN)     events |= EPOLLIN;
	if (_fds[i].revents & POLLOUT)    events |= EPOLLOUT;
	if (_fds[i].revents & POLLERR)    events |= EPOLLERR;
	if (_fds[i].revents & POLLHUP)    events |= EPOLLHUP;

	cgi->handleEvent(events);

	if (cgi->isFinished()) {
		_finalizeCgiResponse(i, cgi_fd);
		return true;
	}
	if (cgi->getState() == CGI_WRITING) {
		_fds[i].events = POLLOUT;
	} else if (cgi->getState() == CGI_READING) {
		_fds[i].events = POLLIN;
	}
	return false;
}

void Server::_registerCgiHandler(int client_fd, CgiHandler *cgi, Client *client) {
	int cgi_fd = cgi->getSocketFd();

	struct pollfd p;
	p.fd = cgi_fd;
	p.revents = 0;
	if (cgi->getState() == CGI_WRITING)
		p.events = POLLOUT | POLLERR | POLLHUP;
	else if (cgi->getState() == CGI_READING)
		p.events = POLLIN | POLLERR | POLLHUP;

	_fds.push_back(p);
	_cgiHandlers[cgi_fd] = cgi;
	_cgiClient[cgi_fd] = client;

	Logger::log(Logger::SERVER,
		"CGI registered (cgi_fd=" + ParseUtils::itoa(cgi_fd)
		+ ", client_fd=" + ParseUtils::itoa(client_fd) + ")");
}

void Server::_finalizeCgiResponse(size_t index, int cgi_fd) {
	CgiHandler* cgi = _cgiHandlers[cgi_fd];
	Client* client = _cgiClient[cgi_fd];

	if (cgi && client) {
		std::string rawCgi = cgi->getOutput();
		Response res;
		res.parseCgiOutput(rawCgi);
		std::string httpResponse = res.buildResponse(client->getServerName(), client->getHttpVersion());
		client->sendResponse(httpResponse);
	}
	close(cgi_fd);

	if (index < _fds.size() && _fds[index].fd == cgi_fd)
		_fds.erase(_fds.begin() + index);
	_cgiHandlers.erase(cgi_fd);
	_cgiClient.erase(cgi_fd);
	delete cgi;
}

void printRequest(ParseHttp parser) {  // for debugging
	std::cout << "Método: " << parser.getRequestMethod() << std::endl;
	std::cout << "URI: " << parser.getUri() << std::endl;
	std::cout << "Path: " << parser.getPath() << std::endl;
	std::cout << "Versão HTTP: " << parser.getHttpVersion() << std::endl;
	std::cout << std::endl;

	std::cout << "=== Todos os Headers ===" << std::endl;
	Request req = parser.buildRequest();
	const std::map<std::string, std::string>& headers = req.getHeaders();
	
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); 
		 it != headers.end(); ++it) {
		std::cout << it->first << ": " << it->second << std::endl;
	}

	std::cout << std::endl;
	std::cout << "=== Cookies ===" << std::endl;
	const std::map<std::string, std::string>& cookies = parser.getCookies();
	
	if (cookies.empty()) {
		std::cout << "(nenhum cookie)" << std::endl;
	} else {
		for (std::map<std::string, std::string>::const_iterator it = cookies.begin(); 
			 it != cookies.end(); ++it) {
			std::cout << it->first << " = " << it->second << std::endl;
		}
	}
}
