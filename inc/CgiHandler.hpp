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

// Status do processamento do CGI
enum CgiState {
    CGI_NOT_STARTED,
    CGI_WRITING_BODY, // Enviando dados para o script (POST)
    CGI_READING,      // Lendo resposta do script
    CGI_FINISHED,
    CGI_ERROR
};

class CgiHandler {
    private:
        pid_t                       _pid;
        int                         _socketFd;      // O lado do pai do socketpair
        CgiState                    _state;
        
        std::string                 _scriptPath;
        std::string                 _executorPath;
        std::map<std::string, std::string> _envMap;
        
        size_t                      _bytesSent;     // Controle de quanto já enviamos
        std::string                 _requestBody;   // O que enviar pro CGI (Stdin do script)
        std::string                 _responseBuffer;// O que recebemos do CGI (Stdout do script)

    public:
        CgiHandler(const Request& req, const LocationConfig* loc, const std::string& scriptPath, const std::string& executor);
        ~CgiHandler();

        // Inicializa o processo (socketpair, fork, execve)
        void        start();

        // Chamado pelo loop principal (epoll/select) quando o FD está pronto
        void        handleEvent(uint32_t events);

        // Verifica se terminou
        bool        isFinished() const;
        
        // Retorna o FD para adicionar no epoll
        int         getFd() const;

        // Processa a resposta final para o objeto Response
        void        buildResponse(Response& res);

    private:
        void        _initEnv(const Request& req, const LocationConfig* loc);
        char**      _createEnvArray() const;
        void        _freeEnvArray(char** envp) const;
        std::string _intToString(int value) const;
};

#endif