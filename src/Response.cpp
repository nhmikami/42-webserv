#include "Response.hpp"

Response::Response(void) : _status(OK) {}

Response::~Response(void) {}

void Response::setStatus(HttpStatus status) {
	_status = status;
}

HttpStatus Response::getStatus(void) const {
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

const std::string& Response::getHeader(const std::string &key) const {
	std::map<std::string, std::string>::const_iterator it = _headers.find(key);
	if (it != _headers.end())
		return it->second;
	static const std::string empty = "";
	return empty;
}

const std::map<std::string, std::string>& Response::getHeaders() const {
	return _headers;
}

std::string Response::buildResponse(void) const {
	std::ostringstream response;

	response << "HTTP/1.1 " << _status << " " << getStatusMessage() << "\r\n";

	if (_headers.find("Date") == _headers.end()) {
		char date[100];
		time_t now = time(0);
		struct tm tm;
		gmtime_r(&now, &tm);
		strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", &tm);
		response << "Date: " << date << "\r\n";
	}

	if (_headers.find("Server") == _headers.end())
		response << "Server: Webserv/42\r\n";

	if (_status == NO_CONTENT) {
		for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
			if (it->first != "Content-Type" && it->first != "Content-Length")
				response << it->first << ": " << it->second << "\r\n";
		}
		response << "\r\n";
		return response.str();
	}

	if (_headers.find("Content-Length") == _headers.end())
		response << "Content-Length: " << _body.size() << "\r\n";

	for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
		response << it->first << ": " << it->second << "\r\n";

	response << "\r\n" << _body;
	return response.str();
}
