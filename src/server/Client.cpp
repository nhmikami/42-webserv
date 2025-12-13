#include "server/Client.hpp"

// Client::Client(void) : _client_fd(-1) {};

// Client::Client(const Client &other) : _client_fd(-1)
// {
// 	(void)other;
// };

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

// Client &Client::operator=(const Client &other)
// {
// 	if (this != &other)
// 	{
// 		_client_fd = -1;
// 	}
// 	return *this;
// };

// std::string Client::receive() 
// {
// 	char buffer[4096];
// 	ssize_t bytes;
// 	std::string header;
// 	Response res;
// 	size_t content_length = 0;
// 	bool headers_complete = false;

// 	while (true) {

// 		bytes = recv(_client_fd, buffer, sizeof(buffer), 0);

// 		if (bytes == 0)
// 			break;

// 		if (bytes < 0) {
// 			res.setStatus(BAD_REQUEST);
// 			sendResponse(res.buildResponse(_server_name, _http_version));
// 			return "";
// 		}

// 		header.append(buffer, static_cast<size_t>(bytes));

// 		if (header.find("\r\n\r\n") != std::string::npos) {
// 			headers_complete = true;
// 			break;
// 		}
// 	}

// 	std::cout << "HEADER!!!" << header << std::endl;

// 	if (headers_complete) {
// 		ParseHttp parse;
// 		parse.initParse(header);
// 		std::string cl_str = parse.getContentLength();
// 		if (!cl_str.empty())
//             content_length = static_cast<size_t>(std::atoi(cl_str.c_str()));
// 	}

// 	std::string body;
// 	// std::cout << "HEADER!!!" << result << std::endl;


// 	size_t header_end = header.find("\r\n\r\n");
//     size_t body_start = header_end + 4;
//     size_t bytes_body_read = (header.size() > body_start) ? (header.size() - body_start) : 0;

// 	while (bytes_body_read < content_length) {
// 		bytes = recv(_client_fd, buffer, sizeof(buffer), 0);

// 		if (bytes == 0)
// 			break;

// 		if (bytes < 0) {
// 			res.setStatus(BAD_REQUEST);
// 			sendResponse(res.buildResponse(_server_name, _http_version));
// 			return "";
// 		}

// 		body.append(buffer, static_cast<size_t>(bytes));
// 		bytes_body_read += static_cast<size_t>(bytes);

// 	}
// 	std::cout << "RESULT!!!" << body << std::endl;
// 	return body;
// };

// std::pair<HttpStatus, ParseHttp> Client::receive() 
// {
//     char buffer[4096];
//     ParseHttp empty_parser;

//     while (true) {
//         size_t
//         ssize_t bytes = recv(_client_fd, buffer, sizeof(buffer), 0);
//         if (bytes > 0) {
//             _recv_buffer.append(buffer, static_cast<size_t>(bytes));
//         } else if (bytes == 0) {
//             // connection closed by peer
//             if (!_recv_buffer.empty()) {
//                 ParseHttp parser;
//                 std::string tmp = _recv_buffer;
//                 HttpStatus status = parser.initParse(tmp);
//                 _recv_buffer.clear();
                
//                 return std::make_pair(status, parser);
//             }
//             return std::make_pair(BAD_REQUEST, empty_parser);
//         } else {
//             Logger::log(Logger::ERROR, "Failed to receive data.");
//             return std::make_pair(BAD_REQUEST, empty_parser);
//         }

//         // Check if we have full headers
//         size_t headers_end = _recv_buffer.find("\r\n\r\n");
//         if (headers_end == std::string::npos) {
//             // need more data for headers
//             continue;
//         }

//         // Try to parse using existing ParseHttp logic to know if body is complete
//         std::string tmp = _recv_buffer; // parser will modify this
//         ParseHttp parser;
//         HttpStatus status = parser.initParse(tmp);

//         if (status == CONTINUE) {
//             // body incomplete, continue reading
//             continue;
//         }

//         // If parser returned OK or an error status, consume the bytes corresponding
//         // to the first request (parser modified tmp to contain leftover beyond the request)
//         size_t consumed = _recv_buffer.size() - tmp.size();
//         _recv_buffer.erase(0, consumed);
//         return std::make_pair(OK, parser);
//     }
// };

std::pair<HttpStatus, ParseHttp> Client::receive()
{
    char buffer[4096];
    ParseHttp empty_parser;

    // 1. Leia até encontrar o fim dos headers
    while (true) {
        size_t headers_end = _recv_buffer.find("\r\n\r\n");
        if (headers_end == std::string::npos) {
            ssize_t bytes = recv(_client_fd, buffer, sizeof(buffer), 0);
            if (bytes > 0) {
                _recv_buffer.append(buffer, static_cast<size_t>(bytes));
                continue;
            } else if (bytes == 0) {
                // conexão fechada pelo cliente
                return std::make_pair(SERVER_ERR, empty_parser);
            } else {
                Logger::log(Logger::ERROR, "Failed to receive data.");
                return std::make_pair(SERVER_ERR, empty_parser);
            }
        }
        break;
    }

    // 2. Parseie apenas os headers
    size_t headers_end = _recv_buffer.find("\r\n\r\n");
    std::string headers_part = _recv_buffer.substr(0, headers_end + 4);
    ParseHttp parser;
    HttpStatus status = parser.parseHeader(headers_part);
    if (status != OK)
        return std::make_pair(status, parser);

    // 3. Descubra o Content-Length
    size_t content_length = 0;
    std::string cl_str = parser.getContentLength();
    if (!cl_str.empty())
        content_length = static_cast<size_t>(std::atoi(cl_str.c_str()));

    // 4. Leia o corpo até ter tudo
    size_t body_start = headers_end + 4;
    while (_recv_buffer.size() < body_start + content_length) {
        ssize_t bytes = recv(_client_fd, buffer, sizeof(buffer), 0);
        if (bytes > 0) {
            _recv_buffer.append(buffer, static_cast<size_t>(bytes));
        } else if (bytes == 0) {
            // conexão fechada antes do corpo completo
            return std::make_pair(BAD_REQUEST, empty_parser);
        } else {
            Logger::log(Logger::ERROR, "Failed to receive data.");
            return std::make_pair(BAD_REQUEST, empty_parser);
        }
    }

    // 5. Parseie o corpo
    std::string body_part = _recv_buffer.substr(body_start, content_length);
    status = parser.parseBody(body_part);
    if (status != OK)
        return std::make_pair(status, parser);

    // 6. Consuma o request do buffer
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
