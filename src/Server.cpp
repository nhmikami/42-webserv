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
	// _address.sin_addr.s_addr = inet_addr(_host);
	if (inet_pton(AF_INET, _host.c_str(), &_address.sin_addr) != 1) {
        // fallback: use INADDR_ANY or handle the error
        _address.sin_addr.s_addr = htonl(INADDR_ANY);
    }
	_address.sin_port = htons(_port);

	return bindServer() && startListen();
};
bool	Server::bindServer() 
{
	if (bind(_server_fd, (struct sockaddr*)&_address, _addlen) < 0)
		return false; //perror("bind failed");
	return true;
};
bool	Server::startListen() 
{
	// 5 é o tamanho máximo da fila de conexões pendentes (backlog) 
	// que o sistema operacional pode armazenar enquanto aguarda 
	// que o servidor aceite cada conexão.
	if (listen(_server_fd, 5) < 0)
		return false;  //perror("listen failed");
	std::cout << "[SERVER] Listening on " << _host << ":" << _port << std::endl;
	return true;
};

// bool	Server::acceptClient(int &client_fd) {};
// void	Server::handleClient(int client_fd) {};
// void	Server::closeServer() {};

void	Server::run() {
	while (true) {
        int client_fd = accept(_server_fd, (struct sockaddr*)&_address, &_addlen);
        if (client_fd < 0) {
            perror("accept failed");
            continue;
        }

        Client client(client_fd);
		std::string request = client.receive();

        if (!request.empty()) {
            std::cout << "[SERVER] Received:\n" << request << std::endl;
        }

        // Aqui eu envio os bytes/buffer para o parsing e recebo uma resposta a qual devolverei para o cliente
        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 13\r\n"
            "\r\n"
            "Hello, World!";

        client.sendResponse(response);
    }
}