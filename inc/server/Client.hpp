#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>

#include "http/Request.hpp"
#include "parse/ParseHttp.hpp"
#include "utils/Logger.hpp"

enum ClientState {
	CLIENT_READING,
	CLIENT_PROCESSING,
	CLIENT_WAITING_CGI,
	CLIENT_WRITING,
	CLIENT_CLOSED
};

class Client {
	private:
		int			_client_fd;
		std::string	_server_name;
		std::string	_http_version;
		std::string	_recv_buffer;
		std::string	_send_buffer;
		size_t		_sent_bytes;
		bool		_keep_alive;
		ParseHttp	_parser;
		Request*	_current_request;
		ClientState	_state;

		Client(void);
		
	public:
		Client(int client_fd);
		~Client(void);

		HttpStatus	receive(void);
		void		initRequest(const std::string& serverName);
		void		queueResponse(const std::string& response);
		bool		sendResponse(void);
		void		prepareForNextRequest(void);
		void		setState(ClientState state);
		
		int			getFd(void);
		std::string	getServerName(void) const;
		std::string	getHttpVersion(void) const;
		ParseHttp&	getParser(void);
		Request*	getCurrentRequest(void);
		ClientState	getState(void) const;
		bool		isKeepAlive(void) const;

};

#endif