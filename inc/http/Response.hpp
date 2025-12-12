#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "config/ServerConfig.hpp"
#include "config/LocationConfig.hpp"
#include "http/Request.hpp"
#include "utils/FileUtils.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>

enum HttpStatus {
	CGI_PENDING = -1,
	ZERO = 0,
	CONTINUE = 100,
	OK = 200,
	CREATED = 201,
	NO_CONTENT = 204,
	MOVED_PERMANENTLY = 301,
	BAD_REQUEST = 400,
	FORBIDDEN = 403,
	NOT_FOUND = 404,
	NOT_ALLOWED = 405,
	TIMEOUT = 408,
	CONFLICT = 409,
	LENGTH_REQUIRED = 411,
	PAYLOAD_TOO_LARGE = 413,
	SERVER_ERR = 500,
	NOT_IMPLEMENTED = 501,
	BAD_GATEWAY = 502,
	SERVICE_UNAVAILABLE = 503,
};

class Response {
	private:
		HttpStatus							_status;
		std::string							_body;
		std::map<std::string, std::string>	_headers;

		std::string			_getErrorPage(int status, const ServerConfig& server, const LocationConfig* location) const;
	
	public:
		Response(void);
		Response(HttpStatus status);
		~Response(void);

		void				setStatus(HttpStatus status);
		void				setBody(const std::string &body);
		void				addHeader(const std::string &key, const std::string &value);

		HttpStatus			getStatus(void) const;
		const std::string	getStatusMessage(void) const;
		const std::string&	getBody(void) const;
		const std::string&	getHeader(const std::string &key) const;
		const std::map<std::string, std::string>&	getHeaders(void) const;

		std::string			buildResponse(void) const;
		std::string			buildResponse(const ServerConfig& server, const Request& request) const;
		HttpStatus			processError(HttpStatus status, const ServerConfig& server, const LocationConfig* location);
		void				parseCgiOutput(const std::string& cgiOutput);
};

#endif