#include "server/Client.hpp"

Client::Client(int client_fd) 
	: _client_fd(client_fd), _sent_bytes(0), _keep_alive(true), _current_request(NULL), _state(CLIENT_READING) {
	_server_name = "WebServ";
	_http_version = "HTTP/1.1";
	Logger::log(Logger::SERVER, "Client connected!");
}

Client::~Client(void) {
	if (_client_fd >= 0) {
		close(_client_fd);
		_client_fd = -1;
		Logger::log(Logger::SERVER, "Connection closed.");
	}
	delete _current_request;
};

HttpStatus Client::receive(void) {
	char buffer[4096];
	ssize_t bytes = recv(_client_fd, buffer, sizeof(buffer), 0);
	
	if (bytes > 0)
		_recv_buffer.append(buffer, bytes);
	else if (bytes == 0)
		return HTTP_CLOSED;
	else
		return HTTP_PENDING;

	if (!_parser.isHeaderComplete()) {
		HttpStatus status = _parser.parseHeader(_recv_buffer);
		if (status != OK)
			return status; 
	}

	return _parser.parseBody(_recv_buffer);
}

void Client::queueResponse(const std::string& response) {
	_send_buffer = response;
	_sent_bytes = 0;
}

bool Client::sendResponse(void) {
	if (_sent_bytes >= _send_buffer.size())
		return true;

	ssize_t sent = send(_client_fd, _send_buffer.c_str() + _sent_bytes, _send_buffer.size() - _sent_bytes, 0);

	if (sent > 0) {
		_sent_bytes += static_cast<size_t>(sent);
		return _sent_bytes == _send_buffer.size();
	}

	return false;
}

void Client::initRequest(const std::string& serverName) {
	if (_current_request) {
		delete _current_request;
		_current_request = NULL;
	}
	_current_request = new Request(_parser.buildRequest());
	_http_version = _current_request->getHttpVersion();
	_server_name = serverName;
	_state = CLIENT_PROCESSING;

	if (_current_request->isKeepAlive())
		_keep_alive = true;
	else
		_keep_alive = false;
}

void Client::prepareForNextRequest(void) {
	if (_current_request) {
		delete _current_request;
		_current_request = NULL;
	}
	_parser.reset();
	_state = CLIENT_READING;
}

int			Client::getFd(void) {
	return _client_fd;
}

std::string	Client::getServerName(void) const {
	return _server_name;
}

std::string	Client::getHttpVersion(void) const {
	return _http_version;
}

ParseHttp& Client::getParser(void) {
	return _parser;
}

Request* Client::getCurrentRequest(void) {
	return _current_request;
}

bool Client::isKeepAlive(void) const {
	return _keep_alive;
}

void Client::setState(ClientState state) {
	_state = state;
}

ClientState Client::getState(void) const {
	return _state;
}
