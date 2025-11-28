#include "Server.hpp"

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
			return false;
		}
	
		address.sin_family = AF_INET;
		if (inet_pton(AF_INET, host.c_str(), &address.sin_addr) != 1) {
			address.sin_addr.s_addr = htonl(INADDR_ANY);
		}
		address.sin_port = htons(port);
	
		if (!(bindServer(server_fd, address, port) && startListen(server_fd, host, port) && addToFDs(server_fd)))
			return false;

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
            continue;
        }

		if (res == 0) continue; 

		for (size_t i = 0; i < _configs.size(); i++) {
			if (_fds[i].revents & POLLIN) {
				int server_fd = _fds[i].fd;
				ServerConfig* config = _fd_to_config[server_fd];
				acceptClient(server_fd, config);
			}
		}

		size_t i = _configs.size();
		while (i < _fds.size()) {
			if (_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
				unhandleClient(i);
			} else if (_fds[i].revents & POLLIN) {
				if (handleClient(i)) i++;
			} else
				i++;
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

bool	Server::handleClient(int i) 
{
	size_t	j = 0;
	int		client_fd = _fds[i].fd;
	Client	*client = NULL;

	client = findClient(&j, client_fd);

	if (!client) return true;

	std::string request = client->receive();
	
	if (request.empty()){
		Logger::log(Logger::SERVER, "Client disconnected (fd=" + ParseUtils::itoa(client_fd) + ")");
		closeClient(i, j, client);
		return false;
	}

	Logger::log(Logger::SERVER, "Received from fd=" + ParseUtils::itoa(client_fd) + ":\n" + request);

	std::string response =
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: 13\r\n"
		"\r\n"
		"Hello, World!";

	client->sendResponse(response);

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

	_fds.erase(_fds.begin() + i);
	_clients.erase(_clients.begin() + j);
	_client_to_config.erase(client_fd);

	delete client;
}
