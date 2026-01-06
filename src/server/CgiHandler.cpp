#include "server/CgiHandler.hpp"
#include "utils/ParseUtils.hpp"

CgiHandler::CgiHandler(const Request& req, const std::string& scriptPath, const std::string& executor)
	: _scriptPath(scriptPath), _executorPath(executor), _pid(-1), _socketFd(-1), _state(CGI_NOT_STARTED), _bytesSent(0), _requestBody(req.getBody()) {
	_initEnv(req);
}

CgiHandler::~CgiHandler(void) {
	if (_pid > 0) {
        kill(_pid, SIGKILL);
        waitpid(_pid, NULL, 0);
        _pid = -1;
    }
    
    if (_socketFd >= 0) {
        close(_socketFd);
        _socketFd = -1;
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

pid_t CgiHandler::getPid(void) const {
	return _pid;
}

int CgiHandler::getSocketFd(void) const {
	return _socketFd;
}

std::string CgiHandler::getOutput(void) const {
	return _responseBuffer;
}

CgiState CgiHandler::getState(void) const {
	return _state;
}

void CgiHandler::start(void) {
	int socks[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, socks) < 0) {
		std::cerr << "[CGI] ERROR: socketpair failed" << std::endl;
		_state = CGI_ERROR;
		return ;
	}

	_pid = fork();
	if (_pid < 0) {
		std::cerr << "[CGI] ERROR: fork failed" << std::endl;
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
		int errFd = open("/tmp/cgi_errors.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (errFd >= 0) {
            dprintf(errFd, "Executor: %s\n", _executorPath.c_str());
            dprintf(errFd, "Script: %s\n", _scriptPath.c_str());
            dprintf(errFd, "Arguments:\n");
            for (size_t i = 0; i < argv.size(); ++i)
                dprintf(errFd, "  argv[%zu]: %s\n", i, argv[i]);
        }

		execve(argv[0], &argv[0], envp);

		if (errFd >= 0)
            dprintf(errFd, "execve failed: %s\n", strerror(errno));
		
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

void CgiHandler::handleEvent(short events) {
	if (_state == CGI_FINISHED || _state == CGI_ERROR) 
		return ;

	if (_state == CGI_WRITING && (events & POLLOUT))
		_handleCgiWrite();
	else if (_state == CGI_READING && (events & POLLIN))
		_handleCgiRead();

	if (events & (POLLHUP | POLLERR)) {
		int status;
		waitpid(_pid, &status, 0);
		if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
			_state = CGI_FINISHED;
		else
			_state = CGI_ERROR;
		return ;
	}
}

void CgiHandler::_handleCgiWrite(void) {
	size_t	remaining = _requestBody.size() - _bytesSent;
	size_t	toWrite = (remaining < CGI_BUF_SIZE) ? remaining : CGI_BUF_SIZE;
	ssize_t	sent;

	if (_bytesSent >= _requestBody.size()) {
		shutdown(_socketFd, SHUT_WR);
		_state = CGI_READING;
		return ;
	}

	sent = write(_socketFd, _requestBody.c_str() + _bytesSent, toWrite);
	if (sent > 0) {
		_bytesSent += sent;
		if (_bytesSent >= _requestBody.size()) {
			shutdown(_socketFd, SHUT_WR);
			_state = CGI_READING;
		}
		return ;
	}

	_state = CGI_ERROR; // sent <= 0
}

void CgiHandler::_handleCgiRead(void) {
	char	buffer[CGI_BUF_SIZE];
	ssize_t	bytesRead;
	int		status;

	bytesRead = read(_socketFd, buffer, sizeof(buffer));
	if (bytesRead > 0) {
		_responseBuffer.append(buffer, bytesRead);
		std::cerr << "[CGI] Read " << bytesRead << " bytes. Total: " << _responseBuffer.size() << std::endl;
		return ;
	}

	if (bytesRead == 0) {
		std::cerr << "[CGI] EOF reached. Buffer size: " << _responseBuffer.size() << std::endl;
        std::cerr << "[CGI] Output: " << _responseBuffer << std::endl;
		pid_t r = waitpid(_pid, &status, WNOHANG);
		if (r == 0) {
			kill(_pid, SIGKILL);
			waitpid(_pid, &status, 0);
		}

		if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
			_state = CGI_FINISHED;
		else
			_state = CGI_ERROR;
		return ;
	}

	std::cerr << "[CGI] ERROR: read failed" << std::endl;
	_state = CGI_ERROR; // bytesRead < 0
}
