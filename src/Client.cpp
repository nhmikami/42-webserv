#include "Client.hpp"

Client::Client(void) : _client_fd(-1)
{
	std::cout << "[SERVER] Client connected!" << std::endl;
};

Client::Client(const Client &other) : _client_fd(other._client_fd) {};

Client::Client(int client_fd) : _client_fd(client_fd) 
{
	std::cout << "[SERVER] Client connected!" << std::endl;
}

Client::~Client(void)
{
	if (_client_fd >= 0)
	{
		close(_client_fd);
		std::cout << "[SERVER] Connection closed." << std::endl;
	}
};

Client &Client::operator=(const Client &other)
{
	if (this != &other)
	{
		_client_fd = other._client_fd;
	}
	return *this;
};

std::string Client::receive() 
{
	char buffer[4096] = {0};
	int bytes = recv(_client_fd, buffer, sizeof(buffer) - 1, 0);
	if (bytes > 0) {
		return std::string(buffer, bytes);
	}
	return "";
};


void		Client::sendResponse(const std::string &response)
{
	send(_client_fd, response.c_str(), response.size(), 0);
};

