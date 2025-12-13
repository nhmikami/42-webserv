#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>

#include "utils/Logger.hpp"
#include "parse/ParseHttp.hpp"
#include "http/Response.hpp"

class Client {
	private:
		int			_client_fd;
		std::string	_server_name;
		std::string	_http_version;
		std::string _recv_buffer;

		Client(void);
		Client(const Client &other);

		Client &operator=(const Client &other);
		
	public:
		Client(int client_fd);
		~Client(void);

		std::pair<HttpStatus, ParseHttp>	receive();
		bool		sendResponse(const std::string &response);

		void		setServerName(const std::string &name);
		void		setHttpVersion(const std::string &version);

		int			getFd();
		std::string	getServerName() const;
		std::string	getHttpVersion() const;
};

#endif