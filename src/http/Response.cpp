#include "http/Response.hpp"

Response::Response(void) : _status(OK) {}

Response::Response(HttpStatus status) : _status(status) {}

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
		case FOUND: return "Found";
		case BAD_REQUEST: return "Bad Request";
		case FORBIDDEN: return "Forbidden";
		case NOT_FOUND: return "Not Found";
		case NOT_ALLOWED: return "Method Not Allowed";
		case TIMEOUT: return "Request Timeout";
		case CONFLICT: return "Conflict";
		case LENGTH_REQUIRED: return "Length Required";
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
	_headers.insert(std::make_pair(ParseUtils::toLower(key), value));
}

const std::string& Response::getHeader(const std::string &key) const {
	std::multimap<std::string, std::string>::const_iterator it = _headers.find(ParseUtils::toLower(key));
	if (it != _headers.end())
		return it->second;
	static const std::string empty = "";
	return empty;
}

const std::multimap<std::string, std::string>& Response::getHeaders() const {
	return _headers;
}

void Response::removeHeader(const std::string &key) {
	_headers.erase(ParseUtils::toLower(key));
}

std::string Response::buildResponse(const std::string& server_name, const std::string& http_version) const {
	std::ostringstream response;
	response << http_version << " " << _status << " " << getStatusMessage() << "\r\n";
	
	if (_headers.find("date") == _headers.end())
		response << "Date: " << _generateDate() << "\r\n";
	response << "Server: " << server_name << "\r\n";

	if (_status != NO_CONTENT && getHeader("content-length").empty()) {
		response << "Content-Length: " << _body.size() << "\r\n";
	}

	for (std::multimap<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
		if (_status == NO_CONTENT && (it->first == "content-type" || it->first == "content-length"))
			continue;
		response << _formatHeaderKey(it->first) << ": " << it->second << "\r\n";
	}
	
	response << "\r\n" << _body;
	return response.str();
}

std::string Response::_formatHeaderKey(std::string key) const {
	bool capitalize = true;
	for (size_t i = 0; i < key.length(); ++i) {
		if (capitalize && islower(key[i]))
			key[i] = toupper(key[i]);
		else if (!capitalize && isupper(key[i]))
			key[i] = tolower(key[i]);
		
		capitalize = (key[i] == '-');
	}
	return key;
}

HttpStatus Response::processError(HttpStatus status, const ServerConfig& server, const LocationConfig* location) {
	_status = status;
	int status_int = static_cast<int>(status);
	std::string error_page = _getErrorPage(status_int, server, location);

	if (!error_page.empty()) {
		std::string path = FileUtils::resolvePath(server.getRoot(), error_page);
		if (FileUtils::exists(path) && FileUtils::isFile(path) && FileUtils::isReadable(path)) {
			struct stat file_stat;
			if (stat(path.c_str(), &file_stat) == 0 && file_stat.st_size > 0) {
				size_t file_size = static_cast<size_t>(file_stat.st_size);
				std::ifstream file(path.c_str(), std::ios::binary);
				if (file) {
					std::vector<char> buffer(file_size);
					file.read(buffer.data(), file_size);
					if (file) {
						setBody(std::string(buffer.begin(), buffer.end()));
						addHeader("content-type", FileUtils::guessMimeType(path));
						return status;
					}
				}
			}
		}
	}
	std::stringstream html;
	html << "<html><head><title>" << static_cast<int>(status) << " " << getStatusMessage() 
		 << "</title></head><body><h1>" << static_cast<int>(status) << " " << getStatusMessage()
		 << "</h1></body></html>\n";

	std::string body = html.str();
	setBody(body);
	addHeader("content-type", "text/html");
	return status;
}

std::string Response::_getErrorPage(int status, const ServerConfig& server, const LocationConfig* location) const {
	std::string	error_path;
	std::map<int, std::string>	error_pages;
	if (location) {
		error_pages = location->getErrorPages();
		if (error_pages.count(status))
			error_path = error_pages[status];
	}
	if (error_path.empty()) {
		error_pages = server.getErrorPages();
		if (error_pages.count(status))
			error_path = error_pages[status];
	}
	return error_path;
}

std::string Response::_generateDate(void) const {
	char date[100];
	time_t now = time(0);
	struct tm tm;

	gmtime_r(&now, &tm);
	strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", &tm);

	return std::string(date);
}

bool Response::parseCgiOutput(const std::string& raw) {
	size_t pos = raw.find("\r\n\r\n");
	size_t offset = 4;
	if (pos == std::string::npos) {
		pos = raw.find("\n\n");
		offset = 2;
	}

	if (pos == std::string::npos)
		return false;

	std::string headersPart = raw.substr(0, pos);
	_body = raw.substr(pos + offset);

	bool has_content_type = false;
	bool has_status = false;

	std::vector<std::string> lines = ParseUtils::split(headersPart, '\n');
	for (size_t i = 0; i < lines.size(); ++i) {
		std::string line = ParseUtils::trim(lines[i]);
		if (line.empty())
			continue;

		if (i == 0 && line.find("HTTP/") == 0) {
			std::vector<std::string> tokens = ParseUtils::split(line, ' ');
			if (tokens.size() >= 2 && ParseUtils::isNumber(tokens[1])) {
				_status = static_cast<HttpStatus>(std::atoi(tokens[1].c_str()));
				has_status = true;
			}
			continue;
		}

		std::pair<std::string, std::string> header = ParseUtils::splitPair(line, ":");
		if (header.second.empty())
			continue;

		std::string key = ParseUtils::toLower(ParseUtils::trim(header.first));
		std::string value = ParseUtils::trim(header.second);
		if (key == "status") {
			std::vector<std::string> statusParts = ParseUtils::split(value, ' ');
			if (!statusParts.empty() && ParseUtils::isNumber(statusParts[0])) {
				_status = static_cast<HttpStatus>(std::atoi(statusParts[0].c_str()));
				has_status = true;
			}
		} else {
			addHeader(key, value);
		}
		if (key == "content-type")
			has_content_type = true;
	}

	if (!has_status && !has_content_type)
		return false;

	if (getHeader("content-length").empty())
		addHeader("content-length", ParseUtils::itoa(_body.size()));

	return true;
}
