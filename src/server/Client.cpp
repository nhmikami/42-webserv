#include "server/Client.hpp"

Client::Client(int client_fd) : _client_fd(client_fd) 
{
	_server_name = "WebServ";
	_http_version = "HTTP/1.1";
	Logger::log(Logger::SERVER, "Client connected!");
}

Client::~Client(void)
{
	if (_client_fd >= 0)
	{
		close(_client_fd);
		Logger::log(Logger::SERVER, "Connection closed.");
	}
};

HttpStatus Client::readHeaders() {
    char buffer[4096];

    while (true) {
        size_t headers_end = _recv_buffer.find("\r\n\r\n");
        if (headers_end == std::string::npos) {
            ssize_t bytes = recv(_client_fd, buffer, sizeof(buffer), 0);
            if (bytes > 0) {
                _recv_buffer.append(buffer, static_cast<size_t>(bytes));
                continue;
            } else if (bytes == 0) {
                return SERVER_ERR;
            } else {
                Logger::log(Logger::ERROR, "Failed to receive data.");
                return SERVER_ERR;
            }
        }
        break;
    }

    return OK;
}

size_t Client::getContentLength(const ParseHttp &parser)
{
    std::string cl_str = parser.getContentLength();
    if (!cl_str.empty())
        return static_cast<size_t>(std::atoi(cl_str.c_str()));
    return 0;
}

HttpStatus Client::readBody(size_t body_start, size_t content_length)
{
    char buffer [4096];

     while (_recv_buffer.size() < body_start + content_length) {
        ssize_t bytes = recv(_client_fd, buffer, sizeof(buffer), 0);
        if (bytes > 0) {
            _recv_buffer.append(buffer, static_cast<size_t>(bytes));
        } else if (bytes == 0) {
            return SERVER_ERR;
        } else {
            Logger::log(Logger::ERROR, "Failed to receive data.");
            return SERVER_ERR;
        }
    }
    return OK;
}

std::pair<HttpStatus, ParseHttp> Client::receive()
{
    ParseHttp parser;

    //HEADERS
    HttpStatus status = readHeaders();
    if (status != OK)
        return std::make_pair(status, parser);

    size_t headers_end = _recv_buffer.find("\r\n\r\n");
    std::string headers_part = _recv_buffer.substr(0, headers_end + 4);

    status = parser.parseHeader(headers_part);
    if (status != OK)
        return std::make_pair(status, parser);
    
    //BODY
    size_t content_length = getContentLength(parser);
    size_t body_start = headers_end + 4;

    status = readBody(body_start, content_length);
    if (status != OK)
        return std::make_pair(status, parser);

    std::string body_part = _recv_buffer.substr(body_start, content_length);
    status = parser.parseBody(body_part);
    if (status != OK)
        return std::make_pair(status, parser);

    _recv_buffer.erase(0, body_start + content_length);
    return std::make_pair(OK, parser);
}

bool		Client::sendResponse(const std::string &response)
{
	size_t		total_sent = 0;
	size_t		to_send = response.size();
	const char*	data = response.c_str();

	while (total_sent < to_send) {
		ssize_t sent = send(_client_fd, data + total_sent, to_send - total_sent, 0);

		if (sent < 0) {
			Logger::log(Logger::ERROR, "Failed to send response.");
			return false;
		}
		if (sent == 0) {
			Logger::log(Logger::ERROR, "Socket closed before response was fully sent.");
			return false;
		}
		total_sent += static_cast<size_t>(sent);
	}

	Logger::log(Logger::SERVER, "Response Sent!");
	return true;
};

int			Client::getFd(){
	return _client_fd;
}

std::string	Client::getServerName() const {
	return _server_name;
}

std::string	Client::getHttpVersion() const {
	return _http_version;
}

void Client::setServerName(const std::string &name) {
	_server_name = name;
}

void Client::setHttpVersion(const std::string &version) {
	_http_version = version;
}
