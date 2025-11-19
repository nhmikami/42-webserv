#include "Client.hpp"

Client::Client(void) : _client_fd(-1) {};

Client::Client(const Client &other) : _client_fd(other._client_fd) {};

Client::Client(int client_fd) : _client_fd(client_fd) 
{
	_logger.log(Logger::SERVER, "Client connected!");
}

Client::~Client(void)
{
	if (_client_fd >= 0)
	{
		close(_client_fd);
		_logger.log(Logger::SERVER, "Connection closed.");
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


bool		Client::sendResponse(const std::string &response)
{
	ssize_t sent = send(_client_fd, response.c_str(), response.size(), 0);
	if (sent < 0) {
		_logger.log(Logger::ERROR, "Failed to send response.");
		return false;
	}
	_logger.log(Logger::SERVER, "Response Sent!");
	return true;
};

int			Client::getFd(){
	return _client_fd;
}

