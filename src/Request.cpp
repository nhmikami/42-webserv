/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cabo-ram <cabo-ram@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/06 15:02:30 by cabo-ram          #+#    #+#             */
/*   Updated: 2025/11/06 15:02:30 by cabo-ram         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Request.hpp"

Request::Request() : _method(UNKNOWN), _uri(""), _path(""),
					 _query(""), _http_version(""), _body("") { }

Request::Request(const Request &other) {
	*this = other;
}

Request &Request::operator=(const Request &other) {
	if (this != &other) {
		this->_method = other._method;
		this->_uri = other._uri;
		this->_path = other._path;
		this->_query = other._query;
		this->_http_version = other._http_version;
		this->_headers = other._headers;
		this->_body = other._body;
	}
	return *this;
}

Request::~Request() { }

void Request::setMethod(RequestMethod m) {
	_method = m;
}

void Request::setUri(const std::string &u) {
	_uri = u;
}

void Request::setPath(const std::string &p) {
	_path = p;
}

void Request::setQuery(const std::string &q) {
	_query = q;
}

void Request::setHttpVersion(const std::string &v) {
	_http_version = v;
}

void Request::setHeaders(const std::map<std::string, std::string> &h) {
	_headers = h;
}

void Request::setBody(const std::string &b) {
	_body = b;
}

void Request::addHeader(const std::string &key, const std::string &value) {
	if (key.empty() || value.empty())
		return ;
	for (size_t i = 0; i < key.size(); ++i) {
		unsigned char c = key[i];
		if (c < 32 || c == 127 || c == ' ' || c == ':' || c == '\r' || c == '\n')
			return ;
	}
	for (size_t i = 0; i < value.size(); ++i) {
		unsigned char c = value[i];
		if (c == '\r' || c == '\n')
			return ;
	}
	_headers[key] = value;
}

RequestMethod Request::getMethod() const {
	return _method;
}

const std::string& Request::getUri() const {
	return _uri;
}

const std::string& Request::getPath() const {
	return _path;
}

const std::string& Request::getQuery() const {
	return _query;
}

const std::string& Request::getHttpVersion() const {
	return _http_version;
}

const std::map<std::string, std::string>& Request::getHeaders() const {
	return _headers;
}

const std::string& Request::getBody() const {
	return _body;
}
		
std::string Request::getHeader(const std::string &key) const {
	std::map<std::string, std::string>::const_iterator it = _headers.find(key);
	if (it != _headers.end())
		return it->second;
	return "";
}

bool Request::hasHeader(const std::string &key) const {
	return _headers.find(key) != _headers.end();
}

size_t Request::getContentLength() const {
	std::string content_length = getHeader("content-length");
	if (content_length.empty())
		return 0;
	return static_cast<size_t>(std::atol(content_length.c_str()));
}

bool Request::isChunked() const {
	std::string transfer_encoding = getHeader("transfer-encoding");
	return transfer_encoding == "chunked";
}

std::string Request::getAcceptedContentTypes() const {
	return getHeader("accept");
}

std::string Request::getConnection() const {
	return getHeader("connection");
}

std::string Request::getUserAgent() const {
	return getHeader("user-agent");
}

std::string Request::getHost() const {
	return getHeader("host");
}

bool Request::isKeepAlive() const {
	std::string connection = getConnection();
	if (_http_version == "HTTP/1.1")
		return connection != "close";
	else
		return connection == "keep-alive";
}

std::string Request::getContentType() const {
	return getHeader("content-type");
}

std::string Request::getQueryParameter(const std::string &key) const {
	if (_query.empty() || key.empty())
		return "";
	
	size_t start = 0;
	size_t end = 0;
	
	while (start < _query.length()) {
		end = _query.find('&', start);
		if (end == std::string::npos)
			end = _query.length();
		
		std::string param = _query.substr(start, end - start);
		size_t equals = param.find('=');
		
		if (equals != std::string::npos) {
			std::string param_key = param.substr(0, equals);
			if (param_key == key)
				return param.substr(equals + 1);
		}
		else if (param == key)
			return "";
		
		start = end + 1;
	}
	return "";
}

std::map<std::string, std::string> Request::getQueryParameters() const {
	std::map<std::string, std::string> params;
	if (_query.empty())
		return params;
	
	size_t start = 0;
	size_t end = 0;
	
	while (start < _query.length()) {
		end = _query.find('&', start);
		if (end == std::string::npos)
			end = _query.length();
		
		std::string param = _query.substr(start, end - start);
		size_t equals = param.find('=');
		
		if (equals != std::string::npos) {
			std::string key = param.substr(0, equals);
			std::string value = param.substr(equals + 1);
			params[key] = value;
		}
		else
			params[param] = "";
		
		start = end + 1;
	}
	
	return params;
}

bool Request::hasQueryParameter(const std::string &key) const {
	const std::map<std::string, std::string>& params = getQueryParameters();
	return params.find(key) != params.end();
}

bool Request::isValidForFileOperation() const {
	if (_path.empty() || _path[0] != '/')
		return false;
	
	return true;
}

std::string Request::getRequestTarget() const {
	if (_query.empty())
		return _path;
	return _path + "?" + _query;
}

bool Request::requiresBody() const {
	return _method == POST;
}
