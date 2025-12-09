#include "server/Server.hpp"

// Server::Server(void) : _server_fd(-1), _addlen(sizeof(_address)), _host("127.0.0.1"), _port(8000)
// {
// 	memset(&_address, 0, sizeof(_address));
// 	if (!startServer())
// 		Logger::log(Logger::ERROR, "Failed to start server.");
// };

// Server::Server(const Server &other) : _server_fd(-1), _addlen(sizeof(_address)), _host(other._host), _port(other._port)
// {
// 	memset(&_address, 0, sizeof(_address));
// 	if (!startServer())
// 		Logger::log(Logger::ERROR, "Failed to start server.");
// };

// Server::Server(std::string host, int port): 
// 	_server_fd(-1), 
// 	_addlen(sizeof(_address)), 
// 	_host(host), 
// 	_port(port)
// {
// 	memset(&_address, 0, sizeof(_address));
// 	if (!startServer())
// 		Logger::log(Logger::ERROR, "Failed to start server.");
// };

// deletar construtores acima //

Server::Server(std::vector<ServerConfig> configs) : _configs(configs)
{
	if (!startServer())
		Logger::log(Logger::ERROR, "Failed to start server.");
}

Server::~Server(void)
{
	for (size_t i = 0; i < _clients.size(); i++)
		delete _clients[i];

	_clients.clear();

	for (size_t i = 0; i < _configs.size(); i++){
		close(_fds[i].fd);
	}
	_fds.clear();

	_fd_to_config.clear();
	_client_to_config.clear();
};

// Server &Server::operator=(const Server &other)
// {
// 	if (this != &other)
// 	{
// 		for (size_t i = 0; i < _clients.size(); i++)
//             delete _clients[i];

//         _clients.clear();
//         _fds.clear();

//         if (_server_fd >= 0)
//             close(_server_fd);

// 		_server_fd = -1;
// 		_addlen = sizeof(_address);
// 		_host = other._host;
// 		_port = other._port;

// 		memset(&_address, 0, sizeof(_address));
// 		if (!startServer())
// 			Logger::log(Logger::ERROR, "Failed to start server.");
// 	}
// 	return *this;
// };

bool	Server::startServer() 
{
	for (size_t i = 0; i < _configs.size(); i++) {
		int server_fd = socket(AF_INET, SOCK_STREAM, 0);
		std::string host = _configs[i].getHost();
		int port = _configs[i].getPort();
		struct sockaddr_in address;

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
	
		address.sin_family = AF_INET;
		if (inet_pton(AF_INET, host.c_str(), &address.sin_addr) != 1) {
			address.sin_addr.s_addr = htonl(INADDR_ANY);
		}
		address.sin_port = htons(port);
	
		if (!(bindServer(server_fd, address, port) && startListen(server_fd, host, port) && addToFDs(server_fd))){
			close(server_fd);
			return false;
		}

		_fd_to_config[server_fd] = &_configs[i];
	}
	return true;
};

bool	Server::bindServer(int server_fd, struct sockaddr_in address, int port)
{
	socklen_t addrlen = sizeof(address);
	if (bind(server_fd, (struct sockaddr*)&address, addrlen) < 0) {
		Logger::log(Logger::ERROR, "Failed to bind server port: " + ParseUtils::itoa(port));
		return false;
	}
	return true;
};

bool	Server::startListen(int server_fd, std::string host, int port) 
{
	if (listen(server_fd, 128) < 0) {
		Logger::log(Logger::ERROR, "Failed to listen.");
		return false;
	}

	Logger::log(Logger::SERVER, "Listening on " + host + ":" + ParseUtils::itoa(port));

	return true;
};

bool	Server::addToFDs(int server_fd)
{
	struct pollfd pollfd;
	pollfd.fd = server_fd;
	pollfd.events = POLLIN;
	pollfd.revents = 0;
	_fds.push_back(pollfd);

	return true;
};

void	Server::run() {
	while (true) {
		int res = poll(_fds.data(), _fds.size(), -1);
		if (res == -1) {
			Logger::log(Logger::ERROR, "Failed to poll.");
			continue ;
		} else if (res == 0) {
			continue ;
		}

		for (size_t i = 0; i < _configs.size(); i++) {
			if (_fds[i].revents & POLLIN) {
				int server_fd = _fds[i].fd;
				ServerConfig* config = _fd_to_config[server_fd];
				acceptClient(server_fd, config);
			}
		}

		size_t i = _configs.size();
		while (i < _fds.size()) {
			int fd = _fds[i].fd;
			if (_cgiHandlers.count(fd)) {
				if (!_handleCgiEvent(i))
					i++;
			} else {
				if (_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
					unhandleClient(i);
				} else if (_fds[i].revents & POLLIN) {
					if (handleClient(i))
						i++;
				} else {
					i++;
				}
			}
		}
	}
}

void	Server::acceptClient(int server_fd, ServerConfig *config)
{
	struct sockaddr_in address;
	socklen_t addrlen = sizeof(address);
	
	int client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen);

	if (client_fd < 0) {
		Logger::log(Logger::ERROR, "Failed to accept client.");
		return;
	}

	addToFDs(client_fd);

	Client *client = new Client(client_fd);
	_clients.push_back(client);
	_client_to_config[client_fd] = config;

	Logger::log(Logger::SERVER, "New client accepted (fd=" + ParseUtils::itoa(client_fd) + ")");
};

Client	*Server::findClient(size_t *j, int client_fd)
{
	while (*j < _clients.size()){
		if (_clients[*j]->getFd() == client_fd) {
			return _clients[*j];
		}
		(*j)++;
	}
	return NULL;
}

ServerConfig *Server::findServerConfig(int client_fd)
{
	std::map<int, ServerConfig*>::iterator it = _client_to_config.find(client_fd);
	if (it == _client_to_config.end()) {
		Logger::log(Logger::ERROR, "Client config not found for fd=" + ParseUtils::itoa(client_fd));
		return NULL;
	}
	return it->second;
}

bool	Server::handleClient(int i) {
	size_t	j = 0;
	int		client_fd = _fds[i].fd;
	Client*	client = findClient(&j, client_fd);
	if (!client)
		return true;

	ServerConfig*	config = findServerConfig(client_fd);
	if (!config) {
		closeClient(i, j, client);
		return false;
	}

	std::string raw_request = client->receive();
	if (raw_request.empty()) {
		Logger::log(Logger::SERVER, "Client disconnected (fd=" + ParseUtils::itoa(client_fd) + ")");
		closeClient(i, j, client);
		return false;
	}
	Logger::log(Logger::SERVER, "Received from fd=" + ParseUtils::itoa(client_fd) + ":\n" + raw_request);

	ParseHttp	parser;
	HttpStatus	status = parser.initParse(raw_request);
	if (status != OK) {
		// tratar erro
		Logger::log(Logger::ERROR, "HTTP parsing failed with status: " + ParseUtils::itoa(status));
		return false;
	}
	Request	request = parser.buildRequest();
	printRequest(parser); // for debugging

	const LocationConfig* location = config->findLocation(FileUtils::normalizePath(request.getPath()));
	if (!_isMethodAllowed(request.getMethodStr(), location)) {
		Logger::log(Logger::ERROR, "Method is not allowed: " + request.getMethodStr());
		return false;
	}

	AMethod* method = NULL;
	if (request.getMethodStr() == "GET")
		method = new MethodGET(request, *config, location);
	else if (request.getMethodStr() == "POST")
		method = new MethodPOST(request, *config, location);
	else if (request.getMethodStr() == "DELETE")
		method = new MethodDELETE(request, *config, location);
	else {
		Logger::log(Logger::ERROR, "Method is not allowed: " + request.getMethodStr());
		return false;
	}
	status = method->handleMethod();
	if (status == CGI_PENDING) {
		CgiHandler* cgi = method->releaseCgiHandler();
		_registerCgiHandler(client_fd, cgi, client);
		Logger::log(Logger::SERVER, "CGI started for client fd=" + ParseUtils::itoa(client_fd));
		delete method;
		return true;
	}

	Response res = method->getResponse();
	std::string response = res.buildResponse();

	Logger::log(Logger::SERVER, "RESPONSE:\n" + response);

	client->sendResponse(response);
	
	delete method;
	return true;
};

void Server::unhandleClient(int i) {
	size_t	j = 0;
	int		client_fd = _fds[i].fd;
	Client	*client = NULL;

	client = findClient(&j, client_fd);

	if (client) {
		Logger::log(Logger::SERVER, "Connection error or hangup (fd=" + ParseUtils::itoa(client_fd) + ")");
		closeClient(i, j, client);
		return ;
	}
	_fds.erase(_fds.begin() + i);
}

void	Server::closeClient(int i, int j, Client *client) {
	int client_fd = client->getFd();

	std::vector<int> cgisToRemove;
	for (std::map<int, Client*>::iterator it = _cgiClient.begin(); it != _cgiClient.end(); ++it) {
		if (it->second == client) {
			int cgi_fd = it->first;
			cgisToRemove.push_back(cgi_fd);
		}
	}

	for (size_t k = 0; k < cgisToRemove.size(); k++) {
		int cgi_fd = cgisToRemove[k];
		CgiHandler* handler = _cgiHandlers[cgi_fd];
		
		for (size_t f = 0; f < _fds.size(); f++) {
			if (_fds[f].fd == cgi_fd) {
				close(cgi_fd);
				_fds.erase(_fds.begin() + f);
				if (f < (size_t)i)
					i--;
				break ;
			}
		}
		delete handler;
		_cgiHandlers.erase(cgi_fd);
		_cgiClient.erase(cgi_fd);
		Logger::log(Logger::ERROR, "Killed orphan CGI for client fd=" + ParseUtils::itoa(client_fd));
	}

	close(client_fd);
	_fds.erase(_fds.begin() + i);
	_clients.erase(_clients.begin() + j);
	_client_to_config.erase(client_fd);

	delete client;
}

bool Server::_handleCgiEvent(size_t i) {
	int cgi_fd = _fds[i].fd;
	CgiHandler *cgi = _cgiHandlers[cgi_fd];
	if (!cgi)
		return false;

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
		_fds[i].events = POLLOUT | POLLERR | POLLHUP;
	} else if (cgi->getState() == CGI_READING) {
		_fds[i].events = POLLIN | POLLERR | POLLHUP;
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

	if (!cgi || !client)
		return ;

	std::string rawCgi = cgi->getOutput();
	Response res;
	res.parseCgiOutput(rawCgi);
	std::string httpResponse = res.buildResponse();
	client->sendResponse(httpResponse);

	close(cgi_fd);
	_fds.erase(_fds.begin() + index);
	_cgiHandlers.erase(cgi_fd);
	_cgiClient.erase(cgi_fd);

	delete cgi;
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