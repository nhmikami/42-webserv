#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

#include <string>
#include <map>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <cerrno>

#include "Request.hpp"
#include "Response.hpp"
#include "LocationConfig.hpp"

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
		CgiHandler(const Request& req, const LocationConfig* loc, const std::string& scriptPath, const std::string& executor);
		~CgiHandler();

		void	start(); // Inicializa o processo (socketpair, fork, execve)
		void	handleEvent(uint32_t events); // Chamado pelo loop principal (epoll/select) quando o FD está pronto
		bool	isFinished() const; // Verifica se terminou
		int		getFd() const; // Retorna o FD para adicionar no epoll/select
		void	buildResponse(Response& res); // Processa a resposta final para o objeto Response

	private:
		void	_initEnv(const Request& req, const LocationConfig* loc);
		char**	_createEnvArray() const;
		void	_freeEnvArray(char** envp) const;
		void	_handleCgiWrite(void);
		void	_handleCgiRead(void);
};

#endif