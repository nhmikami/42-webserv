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
					 _path_info(""), _query(""), _http_version(""), _body("") { }

Request::Request(const Request &other) {
	*this = other;
}

Request &Request::operator=(const Request &other) {
	if (this != &other) {
		this->_method = other._method;
		this->_uri = other._uri;
		this->_path = other._path;
		this->_path_info = other._path_info;
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
	std::string normalized;
	if (ParseUri::normalizePath(p, normalized))
		_path = normalized;
	else
		_path = p;
}

void Request::setPathInfo(const std::string &pi) {
	_path_info = pi;
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

// void Request::addHeader(const std::string &key, const std::string &value) {
// 	if (key.empty() || value.empty())
// 		return ;
// 	for (size_t i = 0; i < key.size(); ++i) {
// 		unsigned char c = key[i];
// 		if (c < 32 || c == 127 || c == ' ' || c == ':' || c == '\r' || c == '\n')
// 			return ;
// 	}
// 	for (size_t i = 0; i < value.size(); ++i) {
// 		unsigned char c = value[i];
// 		if (c == '\r' || c == '\n')
// 			return ;
// 	}
// 	_headers[key] = value;
// }

RequestMethod Request::getMethod() const {
	return _method;
}

const std::string Request::getMethodStr() const {
	switch (_method) {
		case GET:
			return "GET";
		case POST:
			return "POST";
		case DELETE:
			return "DELETE";
		default:
			return "UNKNOWN";
	}
}

const std::string& Request::getUri() const {
	return _uri;
}

const std::string& Request::getPath() const {
	return _path;
}

const std::string& Request::getPathInfo() const {
	return _path_info;
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

std::map<std::string, std::string> Request::getContentTypeParametersMap() const {
	std::map<std::string, std::string> params;
	std::string content_type = getHeader("content-type");
	
	if (content_type.empty())
		return params;
	
	size_t semicolon_pos = content_type.find(';');
	if (semicolon_pos == std::string::npos)
		return params;
	
	std::string params_str = content_type.substr(semicolon_pos + 1);
	
	size_t start = 0;
	while (start < params_str.length()) {
		size_t semicolon = params_str.find(';', start);
		size_t end;
		if (semicolon == std::string::npos)
			end = params_str.length();
		else
			end = semicolon;
		
		std::string param = params_str.substr(start, end - start);
		
		size_t equals = param.find('=');
		if (equals != std::string::npos) {
			std::string key = param.substr(0, equals);
			std::string value = param.substr(equals + 1);
			
			key = ParseUtils::trim(key);
			value = ParseUtils::trim(value);
			
			if (!key.empty())
				params[key] = value;
		}
		
		if (semicolon == std::string::npos)
			break;
		start = semicolon + 1;
	}
	
	return params;
}

// Busca um parâmetro específico na query string
std::string Request::getQueryParameter(const std::string &key) const {
	std::map<std::string, std::string> params = getQueryParametersMap();
	std::map<std::string, std::string>::const_iterator it = params.find(key);
	if (it != params.end())
		return it->second;
	return "";
}

// Retorna todos os parâmetros da query string em um map
std::map<std::string, std::string> Request::getQueryParametersMap() const {
	std::map<std::string, std::string> params;
	if (_query.empty())
		return params;
	
	size_t start = 0;
	size_t end = 0;
	
	// Percorre query separando por '&'
	while (start < _query.length()) {
		// Encontra próximo '&' ou fim da string
		end = _query.find('&', start);
		if (end == std::string::npos)
			end = _query.length();
		
		// Extrai parâmetro completo
		std::string param = _query.substr(start, end - start);
		size_t equals = param.find('=');
		
		if (equals != std::string::npos) {
			// Formato: key=value
			std::string key = param.substr(0, equals);
			std::string value = param.substr(equals + 1);
			
			std::string decoded_key;
			bool key_decoded = ParseUri::urlDecodeQuery(key, decoded_key);
			std::string decoded_value;
			bool value_decoded = ParseUri::urlDecodeQuery(value, decoded_value);
			
			std::string final_key;
			if (key_decoded)
				final_key = decoded_key;
			else
				final_key = key;
			
			std::string final_value;
			if (value_decoded)
				final_value = decoded_value;
			else
				final_value = value;
			
			params[final_key] = final_value;
		}
		else {
			std::string decoded_key;
			if (!ParseUri::urlDecodeQuery(param, decoded_key))
				decoded_key = param;
			params[decoded_key] = "";
		}
		start = end + 1;
	}
	return params;
}

// Verifica se um parâmetro específico existe na query
bool Request::hasQueryParameter(const std::string &key) const {
	if (_query.empty() || key.empty())
		return false;
	
	size_t start = 0;
	while (start < _query.length()) {
		size_t end = _query.find('&', start);
		if (end == std::string::npos)
			end = _query.length();
		
		std::string param = _query.substr(start, end - start);
		size_t equals = param.find('=');
		
		std::string param_key;
		if (equals != std::string::npos)
			param_key = param.substr(0, equals);
		else
			param_key = param;
		
		if (param_key == key)
			return true;
		
		if (end == _query.length())
			break;
		start = end + 1;
	}
	return false;
}

std::string Request::getRequestTarget() const {
	if (_query.empty())
		return _path;
	return _path + "?" + _query;
}

bool Request::requiresBody() const {
	return _method == POST;
}
