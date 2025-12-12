#include "server/Client.hpp"

// Client::Client(void) : _client_fd(-1) {};

// Client::Client(const Client &other) : _client_fd(-1)
// {
// 	(void)other;
// };

Client::Client(int client_fd) : _client_fd(client_fd) 
{
	_server_name = "WebServ";
	_http_version = "HTTP/1.1";
	Logger::log(Logger::SERVER, "Client connected!");
}

Client::~Client(void)
{
	if (_client_fd >= 0)
	{
		close(_client_fd);
		Logger::log(Logger::SERVER, "Connection closed.");
	}
};

// Client &Client::operator=(const Client &other)
// {
// 	if (this != &other)
// 	{
// 		_client_fd = -1;
// 	}
// 	return *this;
// };

std::string Client::receive() 
{
	char buffer[4096];
	ssize_t bytes;
	std::string result;
	Response res;

	while (true) {

		bytes = recv(_client_fd, buffer, sizeof(buffer), 0);

		if (bytes == 0)
			break;

		if (bytes < 0) {
			res.setStatus(BAD_REQUEST);
			sendResponse(res.buildResponse(_server_name, _http_version));
			return "";
		}

		result.append(buffer, static_cast<size_t>(bytes));

		if (result.find("\r\n\r\n") != std::string::npos)
            break;
	}

	return result;
};

bool		Client::sendResponse(const std::string &response)
{
	size_t		total_sent = 0;
	size_t		to_send = response.size();
	const char*	data = response.c_str();

	while (total_sent < to_send) {
		ssize_t sent = send(_client_fd, data + total_sent, to_send - total_sent, 0);

		if (sent < 0) {
			Logger::log(Logger::ERROR, "Failed to send response.");
			return false;
		}
		if (sent == 0) {
			Logger::log(Logger::ERROR, "Socket closed before response was fully sent.");
			return false;
		}
		total_sent += static_cast<size_t>(sent);
	}

	Logger::log(Logger::SERVER, "Response Sent!");
	return true;
};

int			Client::getFd(){
	return _client_fd;
}

std::string	Client::getServerName() const {
	return _server_name;
}

std::string	Client::getHttpVersion() const {
	return _http_version;
}

void Client::setServerName(const std::string &name) {
	_server_name = name;
}

void Client::setHttpVersion(const std::string &version) {
	_http_version = version;
}
