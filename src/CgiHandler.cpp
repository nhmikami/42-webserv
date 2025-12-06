#include "CgiHandler.hpp"
#include "utils/ParseUtils.hpp"


CgiHandler::CgiHandler(const Request& req, const std::string& scriptPath, const std::string& executor)
	: _scriptPath(scriptPath), _executorPath(executor), _pid(-1), _socketFd(-1), _state(CGI_NOT_STARTED), _bytesSent(0), _requestBody(req.getBody()) {
	_initEnv(req);
}

CgiHandler::~CgiHandler(void) {
	if (_socketFd != -1) {
		close(_socketFd);
		_socketFd = -1;
	}
	
	if (_pid > 0) {
		kill(_pid, SIGKILL);
		waitpid(_pid, NULL, 0);
		_pid = -1;
	}
}

void CgiHandler::_initEnv(const Request& req) {
	_envMap["GATEWAY_INTERFACE"] = "CGI/1.1";
	_envMap["SERVER_PROTOCOL"] = "HTTP/1.1";
	_envMap["SERVER_SOFTWARE"] = "WebServ/1.0";
	_envMap["REDIRECT_STATUS"] = "200";
	_envMap["REQUEST_METHOD"] = req.getMethodStr();
	_envMap["QUERY_STRING"] = req.getQuery();
	_envMap["PATH_INFO"] = req.getPathInfo();
	_envMap["SCRIPT_NAME"] = req.getPath();
	_envMap["SCRIPT_FILENAME"] = _scriptPath;
	std::string host = req.getHost();
	if (!host.empty()) {
		size_t pos = host.find(':');
		if (pos != std::string::npos) {
			_envMap["SERVER_NAME"] = host.substr(0, pos);
			_envMap["SERVER_PORT"] = host.substr(pos + 1);
		} else {
			_envMap["SERVER_NAME"] = host;
			_envMap["SERVER_PORT"] = "80";
		}
	}
	const std::map<std::string, std::string>& headers = req.getHeaders();
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
		std::string key = it->first;
		std::string value = it->second;
		std::string transformedKey = ParseUtils::toUpper(ParseUtils::replaceChar(key, '-', '_'));

		if (transformedKey == "CONTENT_LENGTH")
			 _envMap["CONTENT_LENGTH"] = value;
		else if (transformedKey == "CONTENT_TYPE")
			 _envMap["CONTENT_TYPE"] = value;
		else
			_envMap["HTTP_" + transformedKey] = value;
	}
	if (req.getMethodStr() == "POST")
		_envMap["CONTENT_LENGTH"] = ParseUtils::itoa(_requestBody.size());
}

char** CgiHandler::_createEnvArray(void) const {
	char** envp = new char*[_envMap.size() + 1];
	int i = 0;
	for (std::map<std::string, std::string>::const_iterator it = _envMap.begin(); it != _envMap.end(); ++it) {
		std::string tmp = it->first + "=" + it->second;
		envp[i] = new char[tmp.size() + 1];
		std::strcpy(envp[i], tmp.c_str());
		i++;
	}
	envp[i] = NULL;
	return envp;
}

void CgiHandler::_freeEnvArray(char** envp) const {
	if (!envp)
		return ;
	for (int i = 0; envp[i]; ++i)
		delete[] envp[i];
	delete[] envp;
}

int CgiHandler::getFd(void) const {
	return _socketFd;
}

void CgiHandler::start(void) {
	int socks[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, socks) < 0) {
		_state = CGI_ERROR;
		return ;
	}

	_pid = fork();
	if (_pid < 0) {
		close(socks[0]);
		close(socks[1]);
		_state = CGI_ERROR;
		return ;
	}

	if (_pid == 0) {
		if (dup2(socks[1], STDIN_FILENO) < 0)
			exit(1);
		if (dup2(socks[1], STDOUT_FILENO) < 0)
			exit(1);
		close(socks[0]);
		close(socks[1]);

		char** envp = _createEnvArray();
		std::vector<char*> argv;
		argv.push_back(const_cast<char*>(_executorPath.c_str()));
		argv.push_back(const_cast<char*>(_scriptPath.c_str()));
		argv.push_back(NULL);
		execve(argv[0], &argv[0], envp);
		
		_freeEnvArray(envp);
		exit(1);
	}

	close(socks[1]);
	_socketFd = socks[0];
	if (fcntl(_socketFd, F_SETFL, O_NONBLOCK) == -1) {
		close(_socketFd);
		_socketFd = -1;
		_state = CGI_ERROR;
		return ;
	}

	if (!_requestBody.empty()) {
		_state = CGI_WRITING;
	} else {
		_state = CGI_READING;
		shutdown(_socketFd, SHUT_WR);
	}
}

bool CgiHandler::isFinished(void) const {
	return _state == CGI_FINISHED || _state == CGI_ERROR;
}

void CgiHandler::handleEvent(uint32_t events) {
	if (_state == CGI_FINISHED || _state == CGI_ERROR) 
		return ;

	if (_state == CGI_WRITING && (events & EPOLLOUT))
		_handleCgiWrite();

	else if (_state == CGI_READING && (events & (EPOLLIN | EPOLLHUP | EPOLLERR)))
		_handleCgiRead();
}

void CgiHandler::_handleCgiWrite(void) {
	size_t	remaining = _requestBody.size() - _bytesSent;
	size_t	toWrite = (remaining < CGI_BUF_SIZE) ? remaining : CGI_BUF_SIZE;
	ssize_t	sent;
	while (true) {
		sent = write(_socketFd, _requestBody.c_str() + _bytesSent, toWrite);
		if (sent > 0) {
			_bytesSent += sent;
			if (_bytesSent >= _requestBody.size()) {
				shutdown(_socketFd, SHUT_WR);
				_state = CGI_READING;
			}
			break ;
		} else if (sent < 0) {
			if (errno == EINTR)
				continue ;
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return ;
			if (errno == EPIPE) {
				shutdown(_socketFd, SHUT_WR);
				_state = CGI_READING;
				return ;
			}
			_state = CGI_ERROR;
			break ;
		} else {
			_state = CGI_ERROR;
			break ;
		}
	}
}

void CgiHandler::_handleCgiRead(void) {
	char	buffer[CGI_BUF_SIZE];
	ssize_t	bytesRead;
	while (true) {
		bytesRead = read(_socketFd, buffer, sizeof(buffer));
		if (bytesRead > 0) {
			_responseBuffer.append(buffer, bytesRead);
			break ;
		} else if (bytesRead == 0) {
			int status = 0;
			pid_t result = waitpid(_pid, &status, WNOHANG);
			if (result == 0) {
				kill(_pid, SIGKILL);
				waitpid(_pid, &status, 0);
				_state = CGI_FINISHED;
			} else if (result == _pid) {
				if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
					_state = CGI_FINISHED;
				else
					_state = CGI_ERROR;
			} else {
				_state = CGI_ERROR;
			}
			break ;
		} else if (bytesRead < 0) {
			if (errno == EINTR)
				continue ;
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return ;
			_state = CGI_ERROR;
			break ;
		}
	}
}

void CgiHandler::buildResponse(Response& res) {
	(void)res;
	// if (_state == CGI_ERROR) {
	// 	res.setStatus(500);
	// 	res.setBody("Internal Server Error: CGI process failed.");
	// 	return ;
	// }

	// size_t headerEnd = _responseBuffer.find("\r\n\r\n");
	// size_t bodyStart = 0;

	// if (headerEnd != std::string::npos) {
	// 	bodyStart = headerEnd + 4;
	// } else {
	// 	headerEnd = _responseBuffer.find("\n\n");
	// 	if (headerEnd != std::string::npos) {
	// 		bodyStart = headerEnd + 2;
	// 	} else {
	// 		res.setBody(_responseBuffer);
	// 		res.setStatus(200);
	// 		return ;
	// 	}
	// }

	// std::string headers = _responseBuffer.substr(0, headerEnd);
	// std::string body = _responseBuffer.substr(bodyStart);

	// std::stringstream ss(headers);
	// std::string line;
	// while (std::getline(ss, line)) {
	// 	if (!line.empty() && line[line.size() - 1] == '\r')
	// 		line.erase(line.size() - 1);
	// 	if (line.empty())
	// 		continue ;

	// 	size_t sep = line.find(':');
	// 	if (sep != std::string::npos) {
	// 		std::string key = line.substr(0, sep);
	// 		std::string val = line.substr(sep + 1);
			
	// 		size_t first = val.find_first_not_of(" \t");
	// 		if (first != std::string::npos) 
	// 			val = val.substr(first);
	// 		if (key == "Status")
	// 			res.setStatus(std::atoi(val.c_str()));
	// 		else
	// 			res.addHeader(key, val);
	// 	}
	// }

	// res.setBody(body);
	// if (res.getStatus() == 0) res.setStatus(200);
}
