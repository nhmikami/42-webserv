#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

#include <string>
#include <map>
#include <vector>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <algorithm>

#include "http/Request.hpp"
#include "http/Response.hpp"
#include "config/LocationConfig.hpp"

static const size_t CGI_BUF_SIZE = 4096;

enum CgiState {
	CGI_NOT_STARTED,
	CGI_WRITING,
	CGI_READING,
	CGI_FINISHED,
	CGI_ERROR
};

class CgiHandler {
	private:
		std::string							_scriptPath;
		std::string							_executorPath;
		std::map<std::string, std::string>  _envMap;

		pid_t		_pid;
		int			_socketFd;
		CgiState	_state;
		
		size_t				_bytesSent;
		const std::string&	_requestBody;
		std::string			_responseBuffer;

	public:
		CgiHandler(const Request& req, const std::string& scriptPath, const std::string& executor);
		~CgiHandler(void);

		pid_t		getPid(void) const;
		int			getSocketFd(void) const;
		CgiState	getState(void) const;
		std::string	getOutput(void) const;

		void	start(void);
		void	handleEvent(short events);
		bool	isFinished(void) const;

	private:
		void	_initEnv(const Request& req);
		char**	_createEnvArray(void) const;
		void	_freeEnvArray(char** envp) const;
		void	_handleCgiWrite(void);
		void	_handleCgiRead(void);
};

#endif