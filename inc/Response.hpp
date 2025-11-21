/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/12 00:46:40 by marvin            #+#    #+#             */
/*   Updated: 2025/11/12 00:46:40 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "Request.hpp"
#include "ServerConfig.hpp"
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

		const std::string	_guessMimeType(const std::string &path) ;

	public:
		Response(void);
		~Response(void);

		void				setStatus(HttpStatus status);
		void				setBody(const std::string &body);
		void				addHeader(const std::string &key, const std::string &value);
		
		const HttpStatus	getStatus(void) const;
		const std::string	getStatusMessage(void) const;
		const std::string&	getBody(void) const;
		const std::map<std::string, std::string>&	getHeaders(void) const;

		std::string		buildResponse(void) const;
};

Response::Response(void) : _status(OK) {}

Response::~Response(void) {}

void Response::setStatus(HttpStatus status) {
	_status = status;
}

const HttpStatus Response::getStatus(void) const {
	return _status;
}

const std::string Response::getStatusMessage(void) const {
	switch (_status) {
		case CONTINUE: return "Continue";
		case OK: return "OK";
		case CREATED: return "Created";
		case NO_CONTENT: return "No Content";
		case MOVED_PERMANENTLY: return "Moved Permanently";
		case BAD_REQUEST: return "Bad Request";
		case FORBIDDEN: return "Forbidden";
		case NOT_FOUND: return "Not Found";
		case NOT_ALLOWED: return "Method Not Allowed";
		case TIMEOUT: return "Request Timeout";
		case CONFLICT: return "Conflict";
		case PAYLOAD_TOO_LARGE: return "Payload Too Large";
		case SERVER_ERR: return "Internal Server Error";
		case NOT_IMPLEMENTED: return "Not Implemented";
		case BAD_GATEWAY: return "Bad Gateway";
		case SERVICE_UNAVAILABLE: return "Service Unavailable";
		default: return "Unknown";
	}
}

void Response::setBody(const std::string &body) {
	_body = body;
}

const std::string& Response::getBody(void) const {
	return _body;
}

void Response::addHeader(const std::string &key, const std::string &value) {
	_headers[key] = value;
}

const std::map<std::string, std::string>& Response::getHeaders() const {
	return _headers;
}

std::string Response::buildResponse(void) const {
	std::ostringstream response;

	// 1. Status line
	response << "HTTP/1.1 " << _status << " " << getStatusMessage() << "\r\n";

	// 2. Headers
	for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
		response << it->first << ": " << it->second << "\r\n";

	// 3. Separador + body
	response << "\r\n" << _body;

	return response.str();
}

const std::string Response::_guessMimeType(const std::string &path) {
	size_t dot = path.find_last_of('.');
	if (dot == std::string::npos)
		return "application/octet-stream";

	std::string ext = path.substr(dot + 1);

	if (ext == "html" || ext == "htm") return "text/html";
	if (ext == "css") return "text/css";
	if (ext == "js") return "application/javascript";
	if (ext == "png") return "image/png";
	if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
	if (ext == "gif") return "image/gif";
	if (ext == "svg") return "image/svg+xml";
	if (ext == "ico") return "image/x-icon";
	if (ext == "txt") return "text/plain";

	return "application/octet-stream";
}


#endif