#include "server/Server.hpp"

Server::Server(std::vector<ServerConfig> configs) : _configs(configs), _sessions(SESSION_TIMEOUT) {
	srand(static_cast<unsigned int>(time(NULL)));
	startServer();
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
	bool at_least_one_success = false;
	
	for (size_t i = 0; i < _configs.size(); i++) {
		int server_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (server_fd < 0){
			Logger::log(Logger::ERROR, "Socket failed.");
			continue;
		}
		
		int opt = 1;
		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
			Logger::log(Logger::ERROR, "Failed to set SO_REUSEADDR.");
			close(server_fd);
			continue;
		}

		if (fcntl(server_fd, F_SETFL, O_NONBLOCK) < 0) {
			Logger::log(Logger::ERROR, "Failed to set O_NONBLOCK.");
			close(server_fd);
			continue;
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
			continue;
		}
		_fd_to_config[server_fd] = &_configs[i];
		at_least_one_success = true;
	}
	return at_least_one_success;
};

bool	Server::bindServer(int server_fd, struct sockaddr_in address, int port) {
	if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
		Logger::log(Logger::ERROR, "Failed to bind server port " + ParseUtils::itoa(port) + " (already in use). Server block ignored.");
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

bool	Server::addToFDs(int server_fd) {
	struct pollfd pollfd;
	pollfd.fd = server_fd;
	pollfd.events = POLLIN;
	pollfd.revents = 0;
	_fds.push_back(pollfd);

	return true;
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

void	Server::run(void) {
	time_t last_cleanup = std::time(NULL);

	while (true) {
		time_t now = std::time(NULL);
		if (now - last_cleanup >= 60) {
			_sessions.cleanup();
			last_cleanup = now;
		}

		int poll_ret = poll(&_fds[0], _fds.size(), -1);
		if (poll_ret == -1) {
			Logger::log(Logger::ERROR, "Failed to poll.");
			continue ;
		}

		size_t i = 0;
		while (i < _fds.size()) {
			int		fd = _fds[i].fd;
			short	revents = _fds[i].revents;
			bool	advance = true;

			if (revents == 0) {

			} else if (_fd_to_config.count(fd)) {
				if (revents & POLLIN)
					acceptClient(fd, _fd_to_config[fd]);
			} else if (_cgiHandlers.count(fd)) {
				if (_handleCgiEvent(i, revents))
					advance = false;
			} else {
				size_t j = 0;
				Client* client = findClient(&j, fd);
				if (!client) {
					unhandleClient(i);
					advance = false;
				} else if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
					unhandleClient(i);
					advance = false;
				} else if ((revents & POLLOUT) && client->getState() == CLIENT_WRITING) {
					if (client->sendResponse()) {
						Logger::log(Logger::SERVER, "Response sent!");
						if (!resetClient(i, j, client))
							advance = false;
					}
				} else if ((revents & POLLIN) && client->getState() == CLIENT_READING) {
					if (!handleClient(i))
						advance = false;
				}
			}
			if (advance)
				++i;
		}
	}
}

void Server::enablePollOut(int fd) {
	for (size_t i = 0; i < _fds.size(); i++) {
		if (_fds[i].fd == fd) {
			_fds[i].events |= POLLOUT;
			return ;
		}
	}
}

void Server::disablePollOut(size_t i) {
	_fds[i].events &= ~POLLOUT;
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
	
	HttpStatus status = client->receive();
	if (status == HTTP_CLOSED) {
		Logger::log(Logger::SERVER, "Client disconnected (fd=" + ParseUtils::itoa(client_fd) + ")");
		closeClient(j, client);
		return false;
	} else if (status == HTTP_PENDING) {
		return true;
	} else if (status >= BAD_REQUEST) {
		return _completeResponse(status, client, config, NULL, NULL);
	}

	client->initRequest(config->getServerName());
	const LocationConfig* location = config->findLocation(FileUtils::normalizePath(client->getCurrentRequest()->getPath()));
	return _processRequest(*(client->getCurrentRequest()), client, config, location);
}

bool Server::_processRequest(Request& req, Client* cli, ServerConfig* cfg, const LocationConfig* loc) {
	Logger::log(Logger::SERVER, "fd=" + ParseUtils::itoa(cli->getFd()) + " -> " + req.getMethodStr() + " " + req.getPath() + " " + cli->getHttpVersion());
	Response response;
	Session* session = _handleSession(req);
	req.setSession(session);

	if (loc && loc->hasReturn())
		return _processRedirect(static_cast<HttpStatus>(loc->getReturnCode()), cli, loc);

	if (!_isMethodAllowed(req.getMethodStr(), loc))
		return _completeResponse(NOT_ALLOWED, cli, cfg, loc, NULL);

	AMethod* method = NULL;
	if (req.getMethodStr() == "GET")
		method = new MethodGET(req, *cfg, loc);
	else if (req.getMethodStr() == "POST")
		method = new MethodPOST(req, *cfg, loc);
	else if (req.getMethodStr() == "DELETE")
		method = new MethodDELETE(req, *cfg, loc);
	else
		return _completeResponse(NOT_IMPLEMENTED, cli, cfg, loc, method);

	HttpStatus status = method->handleMethod();
	if (status == CGI_PENDING)
		return _processCgi(cli, method);

	return _completeResponse(status, cli, cfg, loc, method);
}

bool Server::_isMethodAllowed(const std::string& method, const LocationConfig* loc) {
	if (!loc)
		return true;
	
	const std::set<std::string>& allowed_methods = loc->getMethods();
	if (allowed_methods.empty())
		return true;
	if (allowed_methods.find(method) == allowed_methods.end())
		return false;

	return true;
}

bool Server::_processRedirect(int code, Client* cli, const LocationConfig* loc) {
	Response res;
	res.setStatus(static_cast<HttpStatus>(code));
	std::string path = loc->getReturnPath();
	if (code >= 300 && code < 400)
		res.addHeader("location", path);
	else if (code >= 400) {
		ServerConfig* cfg = findServerConfig(cli->getFd());
		if (cfg)
			res.processError(static_cast<HttpStatus>(code), *cfg, loc);
	}
	else
		res.setBody(path);

	Request* req = cli->getCurrentRequest();
	if (req && req->getSession())
		res.addHeader("set-cookie", "SESSION_ID=" + req->getSession()->getId() + "; Path=/; HttpOnly");

	cli->queueResponse(res.buildResponse(cli->getServerName(), cli->getHttpVersion()));
	cli->setState(CLIENT_WRITING);
	enablePollOut(cli->getFd());
	return true;
}

bool Server::_completeResponse(HttpStatus status, Client* cli, ServerConfig* cfg, const LocationConfig* loc, AMethod* method) {
	Response res;
	if (method) {
		res = method->getResponse();
		if (status >= BAD_REQUEST)
			res.processError(status, *cfg, loc);
		else
			res.setStatus(status);
	} else {
		res.processError(status, *cfg, loc);
	}

	Request* req = cli->getCurrentRequest();
	if (req && req->getSession()) {
		_processSessionData(res, req->getSession());
		res.addHeader("set-cookie", "SESSION_ID=" + req->getSession()->getId() + "; Path=/; HttpOnly");
	}

	Logger::log(Logger::SERVER, "fd=" + ParseUtils::itoa(cli->getFd()) + " <- " + cli->getHttpVersion() + " " + ParseUtils::itoa(static_cast<int>(status)) + " " + res.getStatusMessage());
	cli->queueResponse(res.buildResponse(cli->getServerName(), cli->getHttpVersion()));
	cli->setState(CLIENT_WRITING);
	enablePollOut(cli->getFd());

	if (method)
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

void Server::closeClient(int j, Client *cli) {
	if (!cli)
		return ;
	int client_fd = cli->getFd();

	std::vector<int> cgi_fds;
	for (std::map<int, Client*>::iterator it = _cgiClient.begin(); it != _cgiClient.end(); ++it) {
		if (it->second == cli)
			cgi_fds.push_back(it->first);
	}

	std::set<int> remove_set;
	remove_set.insert(client_fd);
	for (size_t k = 0; k < cgi_fds.size(); ++k) {
		int cgi_fd = cgi_fds[k];
		remove_set.insert(cgi_fd);
		if (_cgiHandlers.count(cgi_fd)) {
			delete _cgiHandlers[cgi_fd];
			_cgiHandlers.erase(cgi_fd);
		}
		_cgiClient.erase(cgi_fd);
	}

	for (size_t f = 0; f < _fds.size(); ) {
		if (remove_set.find(_fds[f].fd) != remove_set.end()) {
			_fds.erase(_fds.begin() + f);
		} else {
			++f;
		}
	}

	if (j >= 0 && (size_t)j < _clients.size() && _clients[j] == cli) {
		_clients.erase(_clients.begin() + j);
	} else {
		for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
			if (*it == cli) {
				_clients.erase(it);
				break;
			}
		}
	}

	_client_to_config.erase(client_fd);
	delete cli; 
}

bool Server::resetClient(size_t i, size_t j, Client* cli) {
	if (cli->isKeepAlive()) {
		cli->prepareForNextRequest();
		disablePollOut(i);
		return true;
	}
	closeClient(j, cli);
	return false;
}

bool Server::_processCgi(Client* cli, AMethod* method) {
	cli->setState(CLIENT_WAITING_CGI);
	CgiHandler* cgi = method->releaseCgiHandler();
	cgi->start();
	Logger::log(Logger::SERVER, "CGI execution started: pid=" + ParseUtils::itoa(cgi->getPid()));
	_registerCgiHandler(cgi, cli);

	delete method;
	return true;
}

void Server::_registerCgiHandler(CgiHandler *cgi, Client *cli) {
	int cgi_fd = cgi->getSocketFd();

	struct pollfd p;
	p.fd = cgi_fd;
	p.revents = 0;
	if (cgi->getState() == CGI_WRITING)
		p.events = POLLOUT;
	else if (cgi->getState() == CGI_READING)
		p.events = POLLIN;

	_fds.push_back(p);
	_cgiHandlers[cgi_fd] = cgi;
	_cgiClient[cgi_fd] = cli;
}

bool Server::_handleCgiEvent(size_t i, short revents) {
	int cgi_fd = _fds[i].fd;
	if (_cgiHandlers.count(cgi_fd) == 0)
		return false;

	CgiHandler *cgi = _cgiHandlers[cgi_fd];
	cgi->handleEvent(revents);
	if (cgi->isFinished()) {
		_finalizeCgiResponse(i, cgi_fd);
		return true;
	}

	if (cgi->getState() == CGI_WRITING)
		_fds[i].events = POLLOUT | POLLERR | POLLHUP;
	else if (cgi->getState() == CGI_READING)
		_fds[i].events = POLLIN | POLLERR | POLLHUP;
	return false;
}

void Server::_finalizeCgiResponse(size_t i, int cgi_fd) {
	CgiHandler* cgi = _cgiHandlers[cgi_fd];
	Client* cli = _cgiClient[cgi_fd];

	if (cgi && cli) {
		Response res;

		if (cgi->getState() == CGI_ERROR) {
			res.processError(SERVER_ERR, *_client_to_config[cli->getFd()], NULL);
		} else {
			res.parseCgiOutput(cgi->getOutput());
		}

		Request* req = cli->getCurrentRequest();
		if (req && req->getSession()) {
			_processSessionData(res, req->getSession());
			res.addHeader("set-cookie", "SESSION_ID=" + req->getSession()->getId() + "; Path=/; HttpOnly");
		}

		Logger::log(Logger::SERVER, "fd=" + ParseUtils::itoa(cli->getFd()) + " <- " + cli->getHttpVersion() + " " + ParseUtils::itoa(static_cast<int>(res.getStatus())) + " " + res.getStatusMessage());
		cli->queueResponse(res.buildResponse(cli->getServerName(), cli->getHttpVersion()));
		cli->setState(CLIENT_WRITING);
		enablePollOut(cli->getFd());
	}

	_fds.erase(_fds.begin() + i);
	_cgiHandlers.erase(cgi_fd);
	_cgiClient.erase(cgi_fd);
	delete cgi;
}

Session* Server::_handleSession(const Request& request) {
	std::map<std::string, std::string> cookies = request.getCookies();
	std::map<std::string, std::string>::iterator it = cookies.find("SESSION_ID");
	if (it != cookies.end()) {
		Session* s = _sessions.getSession(it->second);
		if (s)
			return s;
	}
	return _sessions.createSession();
}

void Server::_processSessionData(Response& response, Session* session) {
	if (!session)
		return;

	const std::multimap<std::string, std::string>& headers = response.getHeaders();
	std::pair<
		std::multimap<std::string, std::string>::const_iterator, 
		std::multimap<std::string, std::string>::const_iterator
	> range = headers.equal_range("x-session-set");

	for (std::multimap<std::string, std::string>::const_iterator it = range.first; it != range.second; ++it) {
		if (session->size() >= SESSION_MAX_ENTRIES) {
			Logger::log(Logger::ERROR, "Session ID " + session->getId() + " reached max entries limit.");
			break; 
		}

		std::pair<std::string, std::string> key_value = ParseUtils::splitPair(it->second, "=");
		std::string key = ParseUtils::trim(key_value.first);
		std::string value = ParseUtils::trim(key_value.second);
		if (key.empty()) {
			continue;
		}
		if (key.size() > SESSION_MAX_KEY_SIZE) {
			Logger::log(Logger::ERROR, "Session key too long (size: " + ParseUtils::itoa(key.size()) + "). Skipping.");
			continue;
		}
		if (value.size() > SESSION_MAX_VALUE_SIZE) {
			Logger::log(Logger::ERROR, "Session value too long for key '" + key + "' (size: " + ParseUtils::itoa(value.size()) + "). Skipping.");
			continue;
		}
		session->set(key, value);
	}
	response.removeHeader("x-session-set");
}
