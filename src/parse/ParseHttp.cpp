#include "parse/ParseHttp.hpp"

ParseHttp::ParseHttp() : _max_body_size(DEFAULT_CLIENT_MAX_BODY_SIZE) {};

// ParseHttp::ParseHttp(const ParseHttp &other) {
// 	*this = other;
// }

// ParseHttp &ParseHttp::operator=(const ParseHttp &other) {
// 	if (this != &other) {
// 		this->_request_method = other._request_method;
// 		this->_request_uri = other._request_uri;
// 		this->_request_path = other._request_path;
// 		this->_query = other._query;
// 		this->_http_version = other._http_version;
// 		this->_host_header = other._host_header;
// 		this->_user_agent_header = other._user_agent_header;
// 		this->_request_body = other._request_body;
// 		this->_content_length = other._content_length;
// 		this->_content_type = other._content_type;
// 		this->_transfer_encoding = other._transfer_encoding;
// 		this->_connection = other._connection;
// 		this->_accept = other._accept;
// 		this->_all_headers = other._all_headers;
// 	}
// 	return *this;
// }

ParseHttp::~ParseHttp() { }

void ParseHttp::setMaxBodySize(size_t max_body_size) {
	_max_body_size = max_body_size;
}

Request ParseHttp::buildRequest() const {
	Request	req;
	
	req.setMethod(this->_request_method);
	req.setUri(this->_request_uri);
	req.setPath(this->_request_path);
	req.setPathInfo(this->_request_path_info);
	req.setQuery(this->_query);
	req.setHttpVersion(this->_http_version);
	req.setHeaders(this->_all_headers);
	req.setBody(this->_request_body);
	
	return req;
}

RequestMethod ParseHttp::getRequestMethod() const {
	return _request_method;
}

const std::string& ParseHttp::getPath() const {
	return _request_path;
}

const std::string& ParseHttp::getHttpVersion() const {
	return _http_version;
}

const std::string& ParseHttp::getHost() const {
	return _host_header;
}

const std::string& ParseHttp::getUserAgent() const {
	return _user_agent_header;
}

const std::string& ParseHttp::getBody() const {
	return _request_body;
}

const std::string& ParseHttp::getUri() const {
	return _request_uri;
}

const std::map<std::string, std::string>& ParseHttp::getCookies() const {
	return _cookies;
}

// faz o split da request line
bool	ParseHttp::parseRequestLine(const std::string &line,
						std::string &out_method,
						std::string &out_path,
						std::string &out_version) {
	size_t method_end = line.find(" ");
	if (method_end == std::string::npos)
		return false;
	// valida o método
	std::string method = line.substr(0, method_end);
	if (method == "GET" || method == "POST" || method == "DELETE")
		out_method = method;
	else
		return false;
	// extrai o path
	size_t path_start = method_end + 1;
	size_t path_end = line.find(" ", path_start);
	if (path_end == std::string::npos)
		return false;
	
	out_path = line.substr(path_start, path_end - path_start);
	if (out_path.empty())
		return false;
	// verifica versão
	size_t version_start = path_end + 1;
	out_version = line.substr(version_start);
	
	if (!out_version.empty() && out_version[out_version.size() - 1] == '\r')
		out_version.erase(out_version.size() - 1);
	
	if (out_version != "HTTP/1.1")
		return false;
	return true;
}

RequestMethod stringToMethod(const std::string &method) {
	if (method == "GET")
		return GET;
	else if (method == "POST")
		return POST;
	else if (method == "DELETE")
		return DELETE;
	return UNKNOWN;
}

std::ostream& operator<<(std::ostream& os, RequestMethod method) {
	switch(method) {
		case GET:
			os << "GET";
			break;
		case POST:
			os << "POST";
			break;
		case DELETE:
			os << "DELETE";
			break;
		default:
			os << "UNKNOWN";
			break;
	}
	return os;
}

void ParseHttp::toLowerStr(std::string &str) {
	for (size_t i = 0; i < str.size(); ++i)
		str[i] = std::tolower(static_cast<unsigned char>(str[i]));
}

// lê os headers e separa em chave e valor
std::map<std::string,std::string> ParseHttp::parseHeaders(const std::string &headers_block) {
	std::map<std::string,std::string> headers;
	std::istringstream stream(headers_block);
	std::string line;

	while (std::getline(stream, line)) {
		if (line.empty() || line == "\r")
			continue;
		
		size_t colon_pos = line.find(":");
		if (colon_pos == std::string::npos)
			continue;
		
		std::string key = line.substr(0, colon_pos);
		std::string value = line.substr(colon_pos + 1);

		if (!value.empty() && value[value.size() - 1] == '\r')
			value.erase(value.size() - 1);
		
		key = ParseUtils::trim(key);
		value = ParseUtils::trim(value);
		
		toLowerStr(key);

		// Normaliza valores de headers que são case-insensitive
		if (key == "transfer-encoding" || key == "connection")
			toLowerStr(value);

		headers[key] = value;
	}
	return headers;
}

HttpStatus	ParseHttp::initParse(std::string &request) {
	size_t headers_end_pos = request.find("\r\n\r\n");
	if (headers_end_pos == std::string::npos)
		return BAD_REQUEST;
	
	if (headers_end_pos > MAX_HEADER_SIZE)
		return PAYLOAD_TOO_LARGE;
	
	size_t body_start_pos = headers_end_pos + 4;
	std::string headers_block = request.substr(0, headers_end_pos);
	std::string body_buffer = request.substr(body_start_pos);
	request = body_buffer;

	size_t request_line_end = headers_block.find("\r\n");
	if (request_line_end == std::string::npos)
		return BAD_REQUEST;
	
	std::string request_line = headers_block.substr(0, request_line_end);
	std::string method_str, uri_str, version_str;
	
	if (!parseRequestLine(request_line, method_str, uri_str, version_str))
		return BAD_REQUEST;

	std::string path, path_info, query;
	if (!ParseUri::validateUri(uri_str, path, path_info, query))
		return BAD_REQUEST;
	
	std::string normalized_path;
	if (!ParseUri::normalizePath(path, normalized_path))
		return BAD_REQUEST;

	this->_request_uri = uri_str;
	this->_request_path = normalized_path;
	this->_request_path_info = path_info;
	this->_query = query;
	this->_http_version = version_str;
	this->_request_method = stringToMethod(method_str);
	
	std::string headers_only = headers_block.substr(request_line_end + 2);
	std::map<std::string, std::string> headers_map = parseHeaders(headers_only);

	HttpStatus header_validation = ParseHttpValidator::validateHeaders(headers_map);
	if (header_validation != OK)
		return header_validation;

	this->_all_headers = headers_map;

	if (headers_map.find("host") != headers_map.end())
		this->_host_header = headers_map["host"];
	if (headers_map.find("user-agent") != headers_map.end())
		this->_user_agent_header = headers_map["user-agent"];
	if (headers_map.find("content-length") != headers_map.end())
		this->_content_length = headers_map["content-length"];
	if (headers_map.find("content-type") != headers_map.end())
		this->_content_type = headers_map["content-type"];
	if (headers_map.find("transfer-encoding") != headers_map.end())
		this->_transfer_encoding = headers_map["transfer-encoding"];
	if (headers_map.find("connection") != headers_map.end())
		this->_connection = headers_map["connection"];
	if (headers_map.find("accept") != headers_map.end())
		this->_accept = headers_map["accept"];
	if (headers_map.find("cookie") != headers_map.end())
		this->_cookies = ParseCookie::parseCookie(headers_map["cookie"]);

	if (this->_request_method == POST) {
		bool has_content_length = headers_map.find("content-length") != headers_map.end();
		bool has_chunked = headers_map.find("transfer-encoding") != headers_map.end() 
							&& headers_map["transfer-encoding"] == "chunked";
		
		if (!has_content_length && !has_chunked)
			return LENGTH_REQUIRED;
		
		if (has_chunked) {
			HttpStatus status = ParseHttpReader::validateBodyChunked(
				_max_body_size,
				body_buffer,
				this->_request_body
			);
			if (status != OK)
				return status;
			request = body_buffer;
			return OK;
		}
		
		if (has_content_length) {
			HttpStatus status = ParseHttpReader::validateBodyContentLength(
				headers_map["content-length"],
				_max_body_size,
				body_buffer,
				this->_request_body
			);
			if (status != OK)
				return status;
			request = body_buffer;
			return OK;
		}
	}
	return OK;
}