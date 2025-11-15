/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseHttp.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cabo-ram <cabo-ram@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/06 15:02:25 by cabo-ram          #+#    #+#             */
/*   Updated: 2025/11/13 17:28:32 by cabo-ram         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ParseHttp.hpp"
#include "../inc/Response.hpp"
#include "../inc/Request.hpp"

ParseHttp::ParseHttp() { }

ParseHttp::ParseHttp(const ParseHttp &other) {
	*this = other;
}

ParseHttp &ParseHttp::operator=(const ParseHttp &other) {
	if (this != &other) {
		this->request_method = other.request_method;
		this->request_uri = other.request_uri;
		this->request_path = other.request_path;
		this->query = other.query;
		this->http_version = other.http_version;
		this->host_header = other.host_header;
		this->user_agent_header = other.user_agent_header;
		this->request_body = other.request_body;
		this->content_length = other.content_length;
		this->content_type = other.content_type;
		this->transfer_encoding = other.transfer_encoding;
		this->connection = other.connection;
		this->accept = other.accept;
		this->all_headers = other.all_headers;
	}
	return *this;
}

ParseHttp::~ParseHttp() { }

Request ParseHttp::buildRequest() const {
	Request req;
	
	req.setMethod(this->request_method);
	req.setUri(this->request_uri);
	req.setPath(this->request_path);
	req.setQuery(this->query);
	req.setHttpVersion(this->http_version);
	req.setHeaders(this->all_headers);
	req.setBody(this->request_body);
	
	return req;
}

RequestMethod ParseHttp::getRequestMethod() const {
	return request_method;
}

const std::string& ParseHttp::getPath() const {
	return request_path;
}

const std::string& ParseHttp::getHttpVersion() const {
	return http_version;
}

const std::string& ParseHttp::getHost() const {
	return host_header;
}

const std::string& ParseHttp::getUserAgent() const {
	return user_agent_header;
}

const std::string& ParseHttp::getBody() const {
	return request_body;
}

const std::string& ParseHttp::getUri() const {
	return request_uri;
}

// faz o split da request line
bool	ParseHttp::parse_request_line(const std::string &line,
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

// valida regras básicas do URI
bool ParseHttp::validate_uri(const std::string &uri, std::string &path, std::string &query) {
	static const size_t MAX_URI_LEN = 16 * 1024;

	if (uri.empty() || uri[0] != '/')
		return false;
	if (uri.size() > MAX_URI_LEN)
		return false;
	
	if (uri.find('\r') != std::string::npos
		|| uri.find('\n') != std::string::npos
		|| uri.find('\0') != std::string::npos
		|| uri.find('\t') != std::string::npos
		|| uri.find('#') != std::string::npos)
		return false;
	
	for (size_t i = 0; i < uri.size(); ++i) {
		unsigned char c = static_cast<unsigned char>(uri[i]);
		if (c < ' ' && c != '\t')
			return false;
	}
	// separa query após '?'
	size_t pos = uri.find('?');
	if (pos == std::string::npos) {
		path = uri;
		query.clear();
	}
	else {
		path = uri.substr(0, pos);
		query = uri.substr(pos + 1);
		
		if (path.empty() || path == "/")
			path = "/";
		
		if (query.find('?') != std::string::npos)
			return false;
	}
	
	if (path.empty())
		return false;
	
	return true;
}

// verifica e normaliza os paths 
bool ParseHttp::normalize_path(const std::string &raw_path, std::string &normalized_path) {
	if (raw_path.empty() || raw_path[0] != '/')
		return false;
	
	for (size_t i = 0; i < raw_path.size(); ++i) {
		unsigned char c = static_cast<unsigned char>(raw_path[i]);
		if (c < ' ' && c != '\t')
			return false;
		if (c == '<' || c == '>' || c == '|' || c == '"' || c == '\\' || c == '\t')
			return false;
	}
	
	std::vector<std::string> stack;
	std::string s = raw_path;
	size_t i = 1;
	
	while (i <= s.size()) {
		size_t j = s.find('/', i);
		std::string segment;
		if (j == std::string::npos)
			segment = s.substr(i);
		else
			segment = s.substr(i, j - i);
		if (segment.find('\r') != std::string::npos
			|| segment.find('\n') != std::string::npos
			|| segment.find(' ') != std::string::npos)
			return false;
		
		if (segment.empty() || segment == ".") {
		}
		else if (segment == "..") {
			if (!stack.empty())
				stack.pop_back();
		}
		else {
			stack.push_back(segment);
		}
		
		if (j == std::string::npos)
			break;
		i = j + 1;
	}
	
	if (stack.empty()) {
		normalized_path = "/";
	}
	else {
		normalized_path.clear();
		for (size_t k = 0; k < stack.size(); ++k) {
			normalized_path += '/';
			normalized_path += stack[k];
		}
	}
	
	return true;
}

// valida o header host
bool ParseHttp::validate_host_header(const std::string &host) {
	if (host.empty())
		return false;
	
	if (host.size() > 255)
		return false;
	
	size_t colon_pos = host.find(':');
	std::string hostname;
	std::string port;
	
	if (colon_pos != std::string::npos) {
		hostname = host.substr(0, colon_pos);
		port = host.substr(colon_pos + 1);
		
		if (port.empty())
			return false;
		for (size_t i = 0; i < port.size(); ++i) {
			if (!std::isdigit(static_cast<unsigned char>(port[i])))
				return false;
		}
		int port_num = std::atoi(port.c_str());
		if (port_num < 1 || port_num > 65535)
			return false;
	}
	else {
		hostname = host;
		port = "80";
	}
	
	if (hostname.empty())
		return false;
	
	for (size_t i = 0; i < hostname.size(); ++i) {
		unsigned char c = static_cast<unsigned char>(hostname[i]);
		if (!std::isalnum(c) && c != '-' && c != '.')
			return false;
	}
	
	return true;
}

// valida o content_length e se é numérico
bool ParseHttp::validate_content_length(const std::string &content_length_str, size_t &out_length) {
	if (content_length_str.empty())
		return false;
	
	for (size_t i = 0; i < content_length_str.size(); ++i) {
		if (!std::isdigit(static_cast<unsigned char>(content_length_str[i])))
			return false;
	}
	
	char *end_ptr = NULL;
	errno = 0;
	unsigned long value = std::strtoul(content_length_str.c_str(), &end_ptr, 10);
	
	if (errno == ERANGE || value > ULONG_MAX)
		return false;
	
	if (end_ptr != content_length_str.c_str() + content_length_str.size())
		return false;
	
	out_length = static_cast<size_t>(value);
	return true;
}

// normaliza o transfer encoding
bool ParseHttp::validate_transfer_encoding(const std::string &transfer_encoding) {
	if (transfer_encoding.empty())
		return false;

	std::string transf_encod_trim = trim(transfer_encoding);
	
	if (transf_encod_trim == "chunked" || transf_encod_trim == "identity")
		return true;
	
	return false;
}

bool ParseHttp::validate_content_type(const std::string &content_type) {
	if (content_type.empty())
		return false;
	
	size_t slash_pos = content_type.find('/');
	if (slash_pos == std::string::npos || slash_pos == 0)
		return false;
	
	size_t semicolon_pos = content_type.find(';');
	std::string mime_type;
	
	if (semicolon_pos != std::string::npos)
		mime_type = content_type.substr(0, semicolon_pos);
	else
		mime_type = content_type;
	
	mime_type = trim(mime_type);
	
	slash_pos = mime_type.find('/');
	if (slash_pos == std::string::npos || slash_pos == 0 || slash_pos == mime_type.size() - 1)
		return false;
	
	for (size_t i = 0; i < mime_type.size(); ++i) {
		unsigned char c = static_cast<unsigned char>(mime_type[i]);
		if (!std::isalnum(c) && c != '/' && c != '-' && c != '+' && c != '.')
			return false;
	}
	
	return true;
}

// bool ParseHttp::validate_accept(const std::string &accept) {
// 	if (accept.empty())
// 		return false;
	
// 	std::string accept_trimmed = trim(accept);
// 	if (accept_trimmed.empty())
// 		return false;
	
// 	if (accept_trimmed == "*/*")
// 		return true;
	
// 	size_t start = 0;
// 	while (start < accept_trimmed.size()) {
// 		size_t comma_pos = accept_trimmed.find(',', start);
// 		std::string media_type;
		
// 		if (comma_pos == std::string::npos)
// 			media_type = accept_trimmed.substr(start);
// 		else
// 			media_type = accept_trimmed.substr(start, comma_pos - start);
		
// 		media_type = trim(media_type);
		
// 		size_t semicolon_pos = media_type.find(';');
// 		if (semicolon_pos != std::string::npos)
// 			media_type = media_type.substr(0, semicolon_pos);
		
// 		media_type = trim(media_type);
		
// 		if (media_type.empty())
// 			return false;
		
// 		size_t slash_pos = media_type.find('/');
// 		if (slash_pos == std::string::npos || slash_pos == 0 || slash_pos == media_type.size() - 1)
// 			return false;
		
// 		std::string type = media_type.substr(0, slash_pos);
// 		std::string subtype = media_type.substr(slash_pos + 1);
		
// 		for (size_t i = 0; i < type.size(); ++i) {
// 			unsigned char c = static_cast<unsigned char>(type[i]);
// 			if (!std::isalnum(c) && c != '-' && c != '+' && c != '.' && c != '*')
// 				return false;
// 		}
		
// 		for (size_t i = 0; i < subtype.size(); ++i) {
// 			unsigned char c = static_cast<unsigned char>(subtype[i]);
// 			if (!std::isalnum(c) && c != '-' && c != '+' && c != '.' && c != '*')
// 				return false;
// 		}
		
// 		if (comma_pos == std::string::npos)
// 			break;
// 		start = comma_pos + 1;
// 	}
	
// 	return true;
// }

bool ParseHttp::validate_quality_value(const std::string &s) {
	if (s.empty())
		return false;
	for (size_t i = 0; i < s.size(); ++i) {
		if (std::isspace(static_cast<unsigned char>(s[i])))
			return false;
	}

	if (s[0] == '1') {
		if (s.size() == 1)
			return true;
		if (s[1] != '.')
			return false;
		size_t zeros = s.size() - 2;
		if (zeros == 0 || zeros > 3)
			return false;
		for (size_t i = 2; i < s.size(); ++i) {
			if (s[i] != '0')
				return false;
		}
		return true;
	}
	
	if (s[0] == '0') {
		if (s.size() == 1)
			return true;
		if (s[1] != '.')
			return false;
		size_t digits = s.size() - 2;
		if (digits < 1 || digits > 3)
			return false;
		for (size_t i = 2; i < s.size(); ++i) {
			if (!std::isdigit(static_cast<unsigned char>(s[i])))
				return false;
		}
		return true;
	}
	return false;
}

double ParseHttp::q_to_double(const std::string &s) {
	return std::strtod(s.c_str(), NULL);
}

bool ParseHttp::validate_type_token(const std::string &t, bool is_type) {
	(void)is_type;
	if (t.empty())
		return false;
	for (size_t i = 0; i < t.size(); ++i) {
		unsigned char c = static_cast<unsigned char>(t[i]);
		if (c == '*') {
			if (t.size() != 1)
				return false;
			return true;
		}
		if (!std::isalnum(c) && c != '-' && c != '+' && c != '.')
			return false;
	}
	return true;
}

bool ParseHttp::check_params_q(const std::string &params_str, double &out_q, bool &has_q) {
	has_q = false;
	out_q = 1.0;
	if (params_str.empty())
		return true;

	size_t pos = 0;
	while (pos < params_str.size()) {
		size_t next = params_str.find(';', pos);
		std::string param;
		if (next == std::string::npos)
			param = params_str.substr(pos);
		else
			param = params_str.substr(pos, next - pos);

		param = trim(param);
		if (param.empty())
			return false;

		size_t eq = param.find('=');
		if (eq == std::string::npos)
			return false;

		std::string name = param.substr(0, eq);
		std::string value = param.substr(eq + 1);
		name = trim(name);
		value = trim(value);

		for (size_t i = 0; i < name.size(); ++i)
			name[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(name[i])));

		if (name == "q") {
			if (has_q)
				return false;
			if (!validate_quality_value(value))
				return false;
			out_q = q_to_double(value);
			has_q = true;
		}
		if (next == std::string::npos)
			break;
		pos = next + 1;
	}
	return true;
}


bool ParseHttp::validate_accept(const std::string &accept) {
	if (accept.empty())
		return false;
	
	std::string accept_trimmed = trim(accept);
	if (accept_trimmed.empty())
		return false;
	
	if (accept_trimmed == "*/*")
		return true;
	
	size_t start = 0;
	while (start < accept_trimmed.size()) {
		size_t comma_pos = accept_trimmed.find(',', start);
		std::string media_type;
		
		if (comma_pos == std::string::npos)
			media_type = accept_trimmed.substr(start);
		else
			media_type = accept_trimmed.substr(start, comma_pos - start);
		
		media_type = trim(media_type);
		
		std::string media_and_params = media_type;
		std::string params_str;
		size_t semicolon_pos = media_and_params.find(';');
		if (semicolon_pos != std::string::npos) {
			params_str = media_and_params.substr(semicolon_pos + 1);
			media_and_params = media_and_params.substr(0, semicolon_pos);
		}
		media_and_params = trim(media_and_params);
		params_str = trim(params_str);

		if (media_and_params.empty())
			return false;
		
		size_t slash_pos = media_and_params.find('/');
		if (slash_pos == std::string::npos || slash_pos == 0 || slash_pos + 1 >= media_and_params.size())
			return false;
		
		std::string type = media_and_params.substr(0, slash_pos);
		std::string subtype = media_and_params.substr(slash_pos + 1);
		
		if (!validate_type_token(type, true))
			return false;
		if (!validate_type_token(subtype, false))
			return false;

		if (type == "*" && subtype != "*") {
			return false;
		}

		double q_value = 1.0;
		bool has_q = false;
		if (!check_params_q(params_str, q_value, has_q))
			return false;
		
		if (comma_pos == std::string::npos)
			break;
		start = comma_pos + 1;
	}
	
	return true;
}

bool ParseHttp::validate_connection(const std::string &connection) {
	if (connection.empty())
		return false;
	
	std::string connec_trim = trim(connection);
	
	if (connec_trim == "keep-alive" || connec_trim == "close")
		return true;
	
	return false;
}

// Valida regras HTTP mínimas e coerência entre headers antes de aceitar/processar o corpo ou rotear a requisição
HttpStatus ParseHttp::validate_headers(const std::map<std::string, std::string> &headers) {
	if (headers.find("host") == headers.end())
		return BAD_REQUEST;
	
	if (!validate_host_header(headers.find("host")->second))
		return BAD_REQUEST;
	
	bool has_content_length = (headers.find("content-length") != headers.end());
	bool has_transfer_encoding = (headers.find("transfer-encoding") != headers.end());
	
	if (has_content_length && has_transfer_encoding)
		return BAD_REQUEST;
	
	if (has_content_length) {
		size_t content_length;
		if (!validate_content_length(headers.find("content-length")->second, content_length))
			return BAD_REQUEST;
	}
	
	if (has_transfer_encoding) {
		if (!validate_transfer_encoding(headers.find("transfer-encoding")->second))
			return BAD_REQUEST;
	}
	
	if (headers.find("content-type") != headers.end()) {
		if (!validate_content_type(headers.find("content-type")->second))
			return BAD_REQUEST;
	}
	
	if (headers.find("connection") != headers.end()) {
		if (!validate_connection(headers.find("connection")->second))
			return BAD_REQUEST;
	}
	
	if (headers.find("accept") != headers.end()) {
		if (!validate_accept(headers.find("accept")->second))
			return BAD_REQUEST;
	}
	
	return OK;
}

// função auxiliar que remove espaços/tabs da string
std::string trim(const std::string &s) {
	size_t start = s.find_first_not_of(" \t");
	size_t end = s.find_last_not_of(" \t");
	if (start == std::string::npos || end == std::string::npos)
		return "";
	return s.substr(start, end - start + 1);
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

// lê os headers e separa em chave e valor
std::map<std::string,std::string> ParseHttp::parse_headers(const std::string &headers_block) {
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
		
		key = trim(key);
		value = trim(value);
		
		for (size_t i = 0; i < key.size(); ++i)
			key[i] = std::tolower(static_cast<unsigned char>(key[i]));

		// Normaliza valores de headers que são case-insensitive
		if (key == "transfer-encoding" || key == "connection") {
			for (size_t i = 0; i < value.size(); ++i)
				value[i] = std::tolower(static_cast<unsigned char>(value[i]));
		}

		headers[key] = value;
	}
	return headers;
}

// lê os dados do socket até encontrar "\r\n"
bool ParseHttp::read_until_crlf(int client_fd, std::string &buffer, std::string &out_line) {
	static const size_t MAX_LINE = 16 * 1024;
	
	while (true) {
		size_t crlf_pos = buffer.find("\r\n");
		if (crlf_pos != std::string::npos) {
			out_line = buffer.substr(0, crlf_pos);
			buffer.erase(0, crlf_pos + 2);
			return true;
		}
		
		if (buffer.size() > MAX_LINE)
			return false;
		
		char temp[4096];
		ssize_t bytes_received = recv(client_fd, temp, sizeof(temp), 0);
		
		if (bytes_received == 0)
			return false;
		if (bytes_received < 0) {
			if (errno == EINTR)
				continue;
			return false;
		}
		
		buffer.append(temp, static_cast<size_t>(bytes_received));
	}
}

// converte hexadecimal para inteiro (tamanho do chunk)
bool ParseHttp::hex_to_int(const std::string &hex_line, size_t &out_size) {
	std::string hex_str = hex_line;
	
	size_t semicolon_pos = hex_str.find(";");
	if (semicolon_pos != std::string::npos)
		hex_str = hex_str.substr(0, semicolon_pos);
	
	hex_str = trim(hex_str);
	if (hex_str.empty())
		return false;
	
	char *end_ptr = NULL;
	errno = 0;
	unsigned long hex_value = strtoul(hex_str.c_str(), &end_ptr, 16);
	
	if (end_ptr == hex_str.c_str())
		return false;
	
	if (errno == ERANGE)
		return false;
	
	out_size = static_cast<size_t>(hex_value);
	return true;
}

HttpStatus ParseHttp::read_body(int client_fd, size_t content_length) {
	static const size_t MAX_TOTAL = 10 * 1024 * 1024; //10MB
	
	if (content_length == 0) {
		this->request_body.clear();
		return OK;
	}
	
	if (content_length > MAX_TOTAL)
		return PAYLOAD_TOO_LARGE;
	
	char buffer[4096];
	ssize_t bytes_read;
	size_t total_read = 0;

	while (total_read < content_length) {
		size_t bytes_to_read = content_length - total_read;
		if (bytes_to_read > sizeof(buffer))
			bytes_to_read = sizeof(buffer);
		
		bytes_read = recv(client_fd, buffer, bytes_to_read, 0);
		
		if (bytes_read == 0)
			return BAD_REQUEST;
		if (bytes_read < 0) {
			if (errno == EINTR)
				continue;
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return SERVER_ERR;
			return SERVER_ERR;
		}
		
		if (total_read + static_cast<size_t>(bytes_read) > MAX_TOTAL)
			return PAYLOAD_TOO_LARGE;
		
		this->request_body.append(buffer, static_cast<size_t>(bytes_read));
		total_read += static_cast<size_t>(bytes_read);
	}
	return OK;
}

HttpStatus ParseHttp::read_chunked(int client_fd, std::string &buffer) {
	this->request_body.clear();
	static const size_t MAX_TOTAL = 10 * 1024 * 1024;
	size_t total_bytes_read = 0;
	
	while (true) {
		std::string chunk_size_line;
		if (!read_until_crlf(client_fd, buffer, chunk_size_line))
			return BAD_REQUEST;
		
		size_t chunk_size;
		if (!hex_to_int(chunk_size_line, chunk_size))
			return BAD_REQUEST;
		
		if (chunk_size == 0) {
			while (true) {
				std::string trailer_line;
				if (!read_until_crlf(client_fd, buffer, trailer_line))
					return BAD_REQUEST;
				if (trailer_line.empty())
					return OK;
			}
		}
		
		if (total_bytes_read + chunk_size > MAX_TOTAL)
			return PAYLOAD_TOO_LARGE;
		
		while (buffer.size() < chunk_size) {
			char temp[4096];
			ssize_t bytes_received = recv(client_fd, temp, sizeof(temp), 0);
			
			if (bytes_received == 0)
				return BAD_REQUEST;
			if (bytes_received < 0) {
				if (errno == EINTR)
					continue;
				if (errno == EAGAIN || errno == EWOULDBLOCK)
					return SERVER_ERR;
				return SERVER_ERR;
			}
			buffer.append(temp, static_cast<size_t>(bytes_received));
		}
		
		this->request_body.append(buffer.substr(0, chunk_size));
		buffer.erase(0, chunk_size);
		total_bytes_read += chunk_size;
		
		while (buffer.size() < 2) {
			char temp[4096];
			ssize_t bytes_received = recv(client_fd, temp, sizeof(temp), 0);
			
			if (bytes_received == 0)
				return BAD_REQUEST;
			if (bytes_received < 0) {
				if (errno == EINTR)
					continue;
				if (errno == EAGAIN || errno == EWOULDBLOCK)
					return SERVER_ERR;
				return SERVER_ERR;
			}
			buffer.append(temp, static_cast<size_t>(bytes_received));
		}
		
		if (buffer.size() < 2 || buffer[0] != '\r' || buffer[1] != '\n')
			return BAD_REQUEST;
		buffer.erase(0, 2);
	}
}

HttpStatus	ParseHttp::initParse(int client_fd, std::string &remaining_buffer, int max_header_size) {
	char recv_buffer[4096];
	ssize_t bytes_received;
	size_t total_header_size = 0;
	size_t headers_end_pos = 0;

	remaining_buffer.clear();

	while (true) {
		bytes_received = recv(client_fd, recv_buffer, sizeof(recv_buffer), 0);
		
		if (bytes_received == 0)
			return NO_CONTENT;
		if (bytes_received < 0)
			return BAD_REQUEST;
		
		remaining_buffer.append(recv_buffer, bytes_received);
		total_header_size += bytes_received;
		
		if (total_header_size > (size_t)max_header_size)
			return PAYLOAD_TOO_LARGE;
		
		headers_end_pos = remaining_buffer.find("\r\n\r\n");
		if (headers_end_pos != std::string::npos)
			break;
	}
	
	size_t body_start_pos = headers_end_pos + 4;
	std::string headers_block = remaining_buffer.substr(0, headers_end_pos);
	std::string body_buffer = remaining_buffer.substr(body_start_pos);
	remaining_buffer = body_buffer;

	size_t request_line_end = headers_block.find("\r\n");
	if (request_line_end == std::string::npos)
		return BAD_REQUEST;
	
	std::string request_line = headers_block.substr(0, request_line_end);
	std::string method_str, uri_str, version_str;
	
	if (!parse_request_line(request_line, method_str, uri_str, version_str))
		return BAD_REQUEST;

	std::string path, query;
	if (!validate_uri(uri_str, path, query))
		return BAD_REQUEST;
	
	std::string normalized_path;
	if (!normalize_path(path, normalized_path))
		return BAD_REQUEST;

	this->request_uri = uri_str;
	this->request_path = normalized_path;
	this->query = query;
	this->http_version = version_str;
	this->request_method = stringToMethod(method_str);
	
	std::string headers_only = headers_block.substr(request_line_end + 2);
	std::map<std::string, std::string> headers_map = parse_headers(headers_only);

	HttpStatus header_validation = validate_headers(headers_map);
	if (header_validation != OK)
		return header_validation;

	this->all_headers = headers_map;

	if (headers_map.find("host") != headers_map.end())
		this->host_header = headers_map["host"];
	if (headers_map.find("user-agent") != headers_map.end())
		this->user_agent_header = headers_map["user-agent"];
	if (headers_map.find("content-length") != headers_map.end())
		this->content_length = headers_map["content-length"];
	if (headers_map.find("content-type") != headers_map.end())
		this->content_type = headers_map["content-type"];
	if (headers_map.find("transfer-encoding") != headers_map.end())
		this->transfer_encoding = headers_map["transfer-encoding"];
	if (headers_map.find("connection") != headers_map.end())
		this->connection = headers_map["connection"];
	if (headers_map.find("accept") != headers_map.end())
		this->accept = headers_map["accept"];

	if (this->request_method == POST) {
		if (headers_map.find("transfer-encoding") != headers_map.end() && 
			headers_map["transfer-encoding"] == "chunked") {
			return read_chunked(client_fd, remaining_buffer);
		}
		else if (headers_map.find("content-length") != headers_map.end()) {
			size_t content_length;
			if (!validate_content_length(headers_map["content-length"], content_length))
				return BAD_REQUEST;

			this->request_body = remaining_buffer;
			size_t bytes_already_read = remaining_buffer.size();

			if (bytes_already_read >= content_length) {
				this->request_body = this->request_body.substr(0, content_length);
				remaining_buffer = remaining_buffer.substr(content_length);
				return OK;
			}

			size_t bytes_remaining = content_length - bytes_already_read;
			HttpStatus body_status = read_body(client_fd, bytes_remaining);
			if (body_status != OK)
				return body_status;
		}
	}
	return OK;
}