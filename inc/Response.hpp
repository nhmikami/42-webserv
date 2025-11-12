#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "Request.hpp"
#include "Server.hpp"
#include <iostream>
#include <sstream>
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
		HttpStatus	_status_code;
		std::string	_body;
		std::map<std::string, std::string>	_headers;

	public:
		Response(void);
		~Response(void);

		void				setStatus(HttpStatus status);
		HttpStatus			getStatus(void) const;
		std::string			getStatusMessage(void) const;
		void				setHeader(const std::string &key, const std::string &value);
		const std::map<std::string, std::string>&	getHeaders(void) const;
		void				setBody(const std::string &body);
		const std::string&	getBody(void) const;

		void			handleGET(const Request &req, const Server &config);
		void			handlePOST(const Request &req, const Server &config);
		void			handleDELETE(const Request &req, const Server &config);
		std::string		buildResponse(void) const;

};

Response::Response(void) : _status_code(OK) {}

Response::~Response(void) {}

void Response::setStatus(HttpStatus status) {
	_status_code = status;
}

HttpStatus Response::getStatus(void) const {
	return _status_code;
}

std::string Response::getStatusMessage(void) const {
    switch (_status_code) {
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

void Response::setHeader(const std::string &key, const std::string &value) {
    _headers[key] = value;
}

const std::map<std::string, std::string>& Response::getHeaders() const {
    return _headers;
}

std::string Response::buildResponse(void) const {
    std::ostringstream response;

	// 1. Status line
	response << "HTTP/1.1 " << _status_code << " " << getStatusMessage() << "\r\n";

	// 2. Headers
	for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
		response << it->first << ": " << it->second << "\r\n";

	// 3. Separador + body
	response << "\r\n" << _body;

	return response.str();
}


void Response::handleGET(const Request &req, const Server &config) {
	std::string file_path = config.getRoot() + req.getPath();

	// 1. Verifica se o caminho é um diretório
	struct stat path_stat;
	if (stat(file_path.c_str(), &path_stat) == 0 && S_ISDIR(path_stat.st_mode)) {
		// Se for diretório, tenta servir index.html
		if (file_path.back() != '/')
			file_path += '/';
		file_path += "index.html";
	}

	// 2. Tenta abrir o arquivo
	std::ifstream file(file_path.c_str(), std::ios::in | std::ios::binary);
	if (!file.is_open()) {
		setStatus(NOT_FOUND);
		setBody("<h1>404 Not Found</h1>");
		setHeader("Content-Type", "text/html");
		return;
	}

	// 3. Lê o conteúdo do arquivo
	std::ostringstream buffer;
	buffer << file.rdbuf();
	file.close();

	// 4. Monta a resposta
	setStatus(OK);
	setBody(buffer.str());
	setHeader("Content-Type", "text/html");
	setHeader("Content-Length", std::to_string(_body.size()));
	setHeader("Connection", req.isKeepAlive() ? "keep-alive" : "close");
}


#endif