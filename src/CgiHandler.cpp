#include "CgiHandler.hpp"

std::string CgiHandler::_intToString(int value) const {
	std::stringstream ss;
	ss << value;
	return ss.str();
}

CgiHandler::CgiHandler(const Request& req, const LocationConfig* loc, const std::string& scriptPath, const std::string& executor)
	: _pid(-1), _socketFd(-1), _state(CGI_NOT_STARTED), 
	  _scriptPath(scriptPath), _executorPath(executor), 
	  _requestBody(req.getBody()), _bytesSent(0) {
	_initEnv(req, loc);
}

CgiHandler::~CgiHandler() {
	if (_socketFd != -1)
		close(_socketFd);
	
	if (_pid > 0) {
		int status;
		if (waitpid(_pid, &status, WNOHANG) == 0) {
			kill(_pid, SIGKILL);
			waitpid(_pid, &status, 0);
		}
	}
}

void CgiHandler::_initEnv(const Request& req, const LocationConfig* loc) {
	_envMap["SCRIPT_FILENAME"] = _scriptPath;
	_envMap["REQUEST_METHOD"] = req.getMethod();
	_envMap["QUERY_STRING"] = req.getQuery();
	_envMap["SERVER_PROTOCOL"] = "HTTP/1.1";
	_envMap["REDIRECT_STATUS"] = "200";
	_envMap["GATEWAY_INTERFACE"] = "CGI/1.1";
	
	if (req.getMethod() == "POST") {
		_envMap["CONTENT_LENGTH"] = _intToString(_requestBody.size());
		std::string contentType = req.getHeader("Content-Type");
		if (!contentType.empty())
			_envMap["CONTENT_TYPE"] = contentType;
	}
}

char** CgiHandler::_createEnvArray() const {
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

void CgiHandler::start() {
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
		close(socks[0]);
		if (dup2(socks[1], STDIN_FILENO) < 0)
			exit(1);
		if (dup2(socks[1], STDOUT_FILENO) < 0)
			exit(1);
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
	fcntl(_socketFd, F_SETFL, O_NONBLOCK);

	if (!_requestBody.empty())
		_state = CGI_WRITING_BODY;
	else
		_state = CGI_READING;
}

int CgiHandler::getFd() const {
	return _socketFd;
}

bool CgiHandler::isFinished() const {
	return _state == CGI_FINISHED || _state == CGI_ERROR;
}

void CgiHandler::handleEvent(uint32_t events) {
	if (_state == CGI_FINISHED || _state == CGI_ERROR) 
		return ;

	if (_state == CGI_WRITING_BODY) {
		ssize_t sent = write(_socketFd, _requestBody.c_str() + _bytesSent, _requestBody.size() - _bytesSent);
		if (sent > 0) {
			_bytesSent += sent;
			if (_bytesSent >= _requestBody.size()) {
				shutdown(_socketFd, SHUT_WR); 
				_state = CGI_READING;
			}
		} else if (sent < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
			_state = CGI_ERROR;
		}
	}
	
	else if (_state == CGI_READING) {
		char buffer[4096];
		ssize_t bytesRead = read(_socketFd, buffer, sizeof(buffer));
		
		if (bytesRead > 0) {
			_responseBuffer.append(buffer, bytesRead);
		} else if (bytesRead == 0) {
			int status;
			waitpid(_pid, &status, 0);
			if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
				_state = CGI_FINISHED;
			else
				_state = CGI_ERROR;
		} else if (bytesRead < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
			_state = CGI_ERROR;
		}
	}
}

void CgiHandler::buildResponse(Response& res) {
	if (_state == CGI_ERROR) {
		res.setStatus(500);
		return ;
	}

	size_t headerEnd = _responseBuffer.find("\r\n\r\n");
	size_t bodyStart = 0;

	if (headerEnd != std::string::npos) {
		bodyStart = headerEnd + 4;
	} else {
		headerEnd = _responseBuffer.find("\n\n");
		if (headerEnd != std::string::npos) {
			bodyStart = headerEnd + 2;
		} else {
			// Nenhum header encontrado, assume tudo como body (ou erro)
			res.setBody(_responseBuffer);
			res.setStatus(200);
			return ;
		}
	}

	std::string headers = _responseBuffer.substr(0, headerEnd);
	std::string body = _responseBuffer.substr(bodyStart);

	std::stringstream ss(headers);
	std::string line;
	while (std::getline(ss, line)) {
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		if (line.empty())
			continue ;

		size_t sep = line.find(':');
		if (sep != std::string::npos) {
			std::string key = line.substr(0, sep);
			std::string val = line.substr(sep + 1);
			
			// Trim
			size_t first = val.find_first_not_of(" \t");
			if (first != std::string::npos) 
				val = val.substr(first);
			if (key == "Status")
				res.setStatus(std::atoi(val.c_str()));
			else
				res.addHeader(key, val);
		}
	}

	res.setBody(body);
	if (res.getStatus() == 0) res.setStatus(200);
}
