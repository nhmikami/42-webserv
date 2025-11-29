#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <algorithm>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <dirent.h>
#include <map>

enum HttpStatus {
	ZERO,
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
	PAYLOAD_TOO_LARGE = 413,
	SERVER_ERR = 500,
	NOT_IMPLEMENTED = 501,
	BAD_GATEWAY = 502,
	SERVICE_UNAVAILABLE = 503,
};

class Response {
	private:
		HttpStatus	_status;
		std::string	_body;
		std::map<std::string, std::string>	_headers;

	public:
		Response(void);
		~Response(void);

		void				setStatus(HttpStatus status);
		void				setBody(const std::string &body);
		void				addHeader(const std::string &key, const std::string &value);
		
		const HttpStatus	getStatus(void) const;
		const std::string	getStatusMessage(void) const;
		const std::string&	getBody(void) const;
		const std::string&	getHeader(const std::string &key) const;
		const std::map<std::string, std::string>&	getHeaders(void) const;

		std::string		buildResponse(void) const;
};

#endif