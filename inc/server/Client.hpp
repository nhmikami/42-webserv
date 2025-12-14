#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>

#include "utils/Logger.hpp"
#include "parse/ParseHttp.hpp"
#include "http/Request.hpp"
#include "http/Response.hpp"

class Client {
	private:
		int			_client_fd;
		std::string	_server_name;
		std::string	_http_version;
		std::string _recv_buffer;
		Request*	_current_request;

		Client(void);
		
		HttpStatus	readHeaders();
		HttpStatus	readBody(size_t body_start, size_t content_length);
		size_t		getContentLength(const ParseHttp &parser);
		
	public:
		Client(int client_fd);
		~Client(void);

		std::pair<HttpStatus, ParseHttp>	receive();
		bool		sendResponse(const std::string &response);

		void		setServerName(const std::string &name);
		void		setHttpVersion(const std::string &version);

		int			getFd(void);
		std::string	getServerName(void) const;
		std::string	getHttpVersion(void) const;
		Request*	getCurrentRequest(void);
		void		setCurrentRequest(Request* req);
		void		clearCurrentRequest(void);
};

#endif