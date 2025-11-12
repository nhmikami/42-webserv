#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>

class Client {
	private:
		int	_client_fd;

		bool connectToServer();

	public:
		Client(void);
		Client(const Client &other);
		Client(int client_fd);
		// Client(std::string host, int port);
		~Client(void);

		Client &operator=(const Client &other);

		// bool sendRequest(const std::string &request);
		// bool receiveResponse(std::string &response);
		// void disconnect();
		std::string	receive();
		void				sendResponse(const std::string &response);
};

#endif

/*
struct ConnectionMeta {
	int client_fd;
	std::string client_ip;
	uint16_t client_port;
	std::string listening_ip;
	uint16_t listening_port;
	std::vector<Server*> candidates; // servers associados a este socket
};
*/