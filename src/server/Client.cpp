#include "server/Client.hpp"

// Client::Client(void) : _client_fd(-1) {};

// Client::Client(const Client &other) : _client_fd(-1)
// {
// 	(void)other;
// };

Client::Client(int client_fd) : _client_fd(client_fd) 
{
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
	std::string	buffer;
	char		recv_buffer[RECV_BUFFER_SIZE];
	ssize_t		bytes_received;

	buffer.clear();

	while (true) {
		bytes_received = recv(_client_fd, recv_buffer, sizeof(recv_buffer) -1, 0);
		
		if (bytes_received == 0)
			break;
		if (bytes_received < 0)
			Logger::log(Logger::ERROR, "Failed to receive data.");
		
		buffer.append(recv_buffer, bytes_received);
	}

	return buffer;
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

