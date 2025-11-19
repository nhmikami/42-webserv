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

Request::Request() : method(UNKNOWN), uri(""), path(""),
					 query(""), http_version(""), body("") { }

Request::Request(const Request &other) {
	*this = other;
}

Request &Request::operator=(const Request &other) {
	if (this != &other) {
		this->method = other.method;
		this->uri = other.uri;
		this->path = other.path;
		this->query = other.query;
		this->http_version = other.http_version;
		this->headers = other.headers;
		this->body = other.body;
	}
	return *this;
}

Request::~Request() { }

void Request::setMethod(RequestMethod m) {
	method = m;
}

void Request::setUri(const std::string &u) {
	uri = u;
}

void Request::setPath(const std::string &p) {
	path = p;
}

void Request::setQuery(const std::string &q) {
	query = q;
}

void Request::setHttpVersion(const std::string &v) {
	http_version = v;
}

void Request::setHeaders(const std::map<std::string, std::string> &h) {
	headers = h;
}

void Request::setBody(const std::string &b) {
	body = b;
}

void Request::addHeader(const std::string &key, const std::string &value) {
	headers[key] = value;
}

RequestMethod Request::getMethod() const {
	return method;
}

const std::string& Request::getUri() const {
	return uri;
}

const std::string& Request::getPath() const {
	return path;
}

const std::string& Request::getQuery() const {
	return query;
}

const std::string& Request::getHttpVersion() const {
	return http_version;
}

const std::map<std::string, std::string>& Request::getHeaders() const {
	return headers;
}

const std::string& Request::getBody() const {
	return body;
}
		
std::string Request::getHeader(const std::string &key) const {
	std::map<std::string, std::string>::const_iterator it = headers.find(key);
	if (it != headers.end())
		return it->second;
	return "";
}

bool Request::hasHeader(const std::string &key) const {
	return headers.find(key) != headers.end();
}

size_t Request::getContentLength() const {
	std::string content_length = getHeader("content-length");
	if (content_length.empty())
		return 0;
	return static_cast<size_t>(std::atoi(content_length.c_str()));
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
	if (http_version == "HTTP/1.1")
		return connection != "close";
	else
		return connection == "keep-alive";
}

std::string Request::getContentType() const {
	return getHeader("content-type");
}

std::string Request::getQueryParameter(const std::string &key) const {
	std::map<std::string, std::string> params = getQueryParameters();
	std::map<std::string, std::string>::const_iterator it = params.find(key);
	if (it != params.end())
		return it->second;
	return "";
}

std::map<std::string, std::string> Request::getQueryParameters() const {
	std::map<std::string, std::string> params;
	if (query.empty())
		return params;
	
	size_t start = 0;
	size_t end = 0;
	
	while (start < query.length()) {
		end = query.find('&', start);
		if (end == std::string::npos)
			end = query.length();
		
		std::string param = query.substr(start, end - start);
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
	return !getQueryParameter(key).empty();
}

bool Request::isValidForFileOperation() const {
	if (path.empty() || path[0] != '/')
		return false;
	
	if (path.find("..") != std::string::npos)
		return false;
	
	for (size_t i = 0; i < path.length(); ++i) {
		char c = path[i];
		if (c < 32 || c == 127)
			return false;
		if (c == '<' || c == '>' || c == '|' || c == '"' || c == '\\')
			return false;
	}
	
	return true;
}

std::string Request::getRequestTarget() const {
	if (query.empty())
		return path;
	return path + "?" + query;
}

bool Request::requiresBody() const {
	return method == POST;
}
