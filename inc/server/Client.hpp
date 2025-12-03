#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>

#include "utils/Logger.hpp"

class Client {
	private:
		int		_client_fd;

		Client(void);
		Client(const Client &other);

		Client &operator=(const Client &other);
		
	public:
		Client(int client_fd);
		~Client(void);

		std::string	receive();
		bool		sendResponse(const std::string &response);
		int			getFd();
};

#endif