#include "Server.hpp"

Server::Server(void) : _server_fd(-1), _addlen(sizeof(_address)), _host("127.0.0.1"), _port(8000)
{
	memset(&_address, 0, sizeof(_address));
	if (!startServer())
		perror("Failed to start server.");

	
};

Server::Server(const Server &other) : 
	_server_fd(other._server_fd),
	_address(other._address),
	_addlen(other._addlen),
	_host(other._host), 
	_port(other._port)
{};

Server::Server(std::string host, int port): 
	_server_fd(-1), 
	_addlen(sizeof(_address)), 
	_host(host), 
	_port(port) 
{};

Server::~Server(void)
{
	for (size_t i = 0; i < _clients.size(); i++)
		delete _clients[i];

	_clients.clear();
    _fds.clear();

	if (_server_fd >= 0)
		close(_server_fd);
};

Server &Server::operator=(const Server &other)
{
	if (this != &other)
	{
		_server_fd = other._server_fd;
		_address = other._address;
		_addlen = other._addlen;
		_host = other._host;
		_port = other._port;
	}
	return *this;
};

bool	Server::startServer() 
{
	_server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_server_fd < 0) //perror("socket failed");
		return false;

	int opt = 1;
	setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	_address.sin_family = AF_INET;
	if (inet_pton(AF_INET, _host.c_str(), &_address.sin_addr) != 1) {
        _address.sin_addr.s_addr = htonl(INADDR_ANY);
    }
	_address.sin_port = htons(_port);

	return bindServer() && startListen() && addToFDs(_server_fd);
};
bool	Server::bindServer() 
{
	if (bind(_server_fd, (struct sockaddr*)&_address, _addlen) < 0)
		return false; //perror("bind failed");
	return true;
};
bool	Server::startListen() 
{
	if (listen(_server_fd, 128) < 0)
		return false;  //perror("listen failed");
	std::cout << "[SERVER] Listening on " << _host << ":" << _port << std::endl;
	return true;
};

bool	Server::addToFDs(int fd)
{
	struct pollfd pollfd;
	pollfd.fd = fd;
	pollfd.events = POLLIN;
	_fds.push_back(pollfd);

	return true;
};

void	Server::run() {

	while (true) {
		int res = poll(_fds.data(), _fds.size(), -1);

		if (res == -1) {
            perror("poll failed");
            continue;
        }

		if (res == 0) continue; 

		if (_fds[0].revents & POLLIN)
			acceptClient();

		size_t i = 1;
		while (i < _fds.size()) {
			if (_fds[i].revents & POLLIN) {
				if (handleClient(i)) i++;
			} else
				i++;
		}
    }
}

void	Server::acceptClient()
{
	int client_fd = accept(_server_fd, (struct sockaddr*)&_address, &_addlen);

	if (client_fd < 0) {
        perror("accept failed");
        return;
    }

	addToFDs(client_fd);

	Client *client = new Client(client_fd);
	_clients.push_back(client);

	std::cout << "[SERVER] New client accepted (fd=" << client_fd << ")" << std::endl;
	
};
bool	Server::handleClient(int i) 
{
	size_t	j = 0;
	int		fd = _fds[i].fd;
	Client	*client = NULL;

	while (j < _clients.size()){
		if (_clients[j]->getFd() == fd) {
			client = _clients[j];
			break;
		}
		j++;
	}

	if (!client) return true;

	std::string request = client->receive();
	
	if (request.empty()){
		std::cout << "[SERVER] Client disconnected (fd=" << fd << ")" << std::endl;
		closeClient(i, j, client);
		return false;
	}

	std::cout << "[SERVER] Received from fd=" << fd << ":\n" << request << std::endl;

	std::string response =
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: text/plain\r\n"
		"Content-Length: 13\r\n"
		"\r\n"
		"Hello, World!";

	client->sendResponse(response);

	closeClient(i, j, client);
	return true;
};

void	Server::closeClient(int i, int j, Client *client) {
	_fds.erase(_fds.begin() + i);
	_clients.erase(_clients.begin() + j);
	delete client;
}
