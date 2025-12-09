#include "Response.hpp"

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
		case BAD_REQUEST: return "Bad Request";
		case FORBIDDEN: return "Forbidden";
		case NOT_FOUND: return "Not Found";
		case NOT_ALLOWED: return "Method Not Allowed";
		case TIMEOUT: return "Request Timeout";
		case CONFLICT: return "Conflict";
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
	_headers[key] = value;
}

const std::string& Response::getHeader(const std::string &key) const {
	std::map<std::string, std::string>::const_iterator it = _headers.find(key);
	if (it != _headers.end())
		return it->second;
	static const std::string empty = "";
	return empty;
}

const std::map<std::string, std::string>& Response::getHeaders() const {
	return _headers;
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
						addHeader("Content-Type", FileUtils::guessMimeType(path));
						return status;
					}
				}
			}
		}
	}
	std::stringstream html;
	html << "<html><head><title>" << static_cast<int>(status) << " " << getStatusMessage() 
		 << "</title></head><body><h1>" << static_cast<int>(status) << " " << getStatusMessage()
		 << "</h1></body></html>";

	std::string body = html.str();
	setBody(body);
	addHeader("Content-Type", "text/html");
	return status;
}

std::string Response::buildResponse(void) const {
	std::ostringstream response;

	response << "HTTP/1.1 " << _status << " " << getStatusMessage() << "\r\n";

	if (_headers.find("Date") == _headers.end()) {
		char date[100];
		time_t now = time(0);
		struct tm tm;
		gmtime_r(&now, &tm);
		strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", &tm);
		response << "Date: " << date << "\r\n";
	}

	if (_headers.find("Server") == _headers.end())
		response << "Server: Webserv/42\r\n";

	if (_status == NO_CONTENT) {
		for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
			if (it->first != "Content-Type" && it->first != "Content-Length")
				response << it->first << ": " << it->second << "\r\n";
		}
		response << "\r\n";
		return response.str();
	}

	if (_headers.find("Content-Length") == _headers.end())
		response << "Content-Length: " << _body.size() << "\r\n";

	for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
		response << it->first << ": " << it->second << "\r\n";

	response << "\r\n" << _body;
	return response.str();
}

void Response::parseCgiOutput(const std::string& rawOutput) {
    if (rawOutput.empty()) {
        _body.clear();
        return ;
    }

    // Encontra separador de headers/body (prefere CRLF CRLF)
    size_t headerEnd = rawOutput.find("\r\n\r\n");
    size_t sepLen = 4;
    if (headerEnd == std::string::npos) {
        headerEnd = rawOutput.find("\n\n");
        sepLen = 2;
    }
    // Se não encontrou separador, trata tudo como body
    if (headerEnd == std::string::npos) {
        setBody(rawOutput);
        return;
    }

    std::string headersPart = rawOutput.substr(0, headerEnd);
    std::string bodyPart = rawOutput.substr(headerEnd + sepLen);

    // Split linhas (suporta CRLF ou LF)
    std::vector<std::string> lines;
    size_t pos = 0;
    while (pos < headersPart.size()) {
        size_t next = headersPart.find("\r\n", pos);
        if (next == std::string::npos) {
            next = headersPart.find('\n', pos);
            if (next == std::string::npos) {
                lines.push_back(headersPart.substr(pos));
                break;
            }
            lines.push_back(headersPart.substr(pos, next - pos));
            pos = next + 1;
        } else {
            lines.push_back(headersPart.substr(pos, next - pos));
            pos = next + 2;
        }
    }

    // utilitários simples de trim
    auto ltrim = [](std::string &s) {
        size_t i = 0;
        while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
        if (i) s.erase(0, i);
    };
    auto rtrim = [](std::string &s) {
        size_t i = s.size();
        while (i > 0 && std::isspace(static_cast<unsigned char>(s[i - 1]))) --i;
        if (i != s.size()) s.erase(i);
    };
    auto trim = [&ltrim, &rtrim](std::string &s) { ltrim(s); rtrim(s); };

    // Processa linhas de headers
    for (size_t i = 0; i < lines.size(); ++i) {
        std::string line = lines[i];
        trim(line);
        if (line.empty())
            continue;

        // Caso: primeira linha pode ser "HTTP/1.1 200 OK"
        if (i == 0 && line.size() > 5 && line.substr(0, 5) == "HTTP/") {
            // tenta extrair o código de status
            std::istringstream ss(line);
            std::string proto;
            int code = 0;
            ss >> proto >> code;
            if (code > 0)
                _status = static_cast<HttpStatus>(code);
            continue;
        }

        // Header normal "Name: value"
        size_t colon = line.find(':');
        if (colon == std::string::npos) {
            // linha inválida, ignora
            continue;
        }
        std::string name = line.substr(0, colon);
        std::string value = line.substr(colon + 1);
        trim(name);
        trim(value);

        // Header especial "Status: 200 OK" -> define status
        std::string nameLower = name;
        for (size_t k = 0; k < nameLower.size(); ++k) nameLower[k] = std::tolower(static_cast<unsigned char>(nameLower[k]));
        if (nameLower == "status") {
            // extrai número do início do value
            std::istringstream ss(value);
            int code = 0;
            ss >> code;
            if (code > 0)
                _status = static_cast<HttpStatus>(code);
            continue;
        }

        // Adiciona header ao response (sobrescreve se já existir)
        addHeader(name, value);
    }

    // Define body e garante Content-Length correto
    setBody(bodyPart);
    addHeader("Content-Length", std::to_string(_body.size()));

    // Se CGI não forneceu Content-Type, mantém o que já estava ou adiciona "text/plain"
    if (getHeader("Content-Type").empty())
        addHeader("Content-Type", "text/plain");
}

	// void Response::parseCgiOutput(const std::string& rawOutput) {
	// 	(void)rawOutput;
    // // 1. Separar cabeçalho do corpo
    // size_t headerEnd = rawOutput.find("\r\n\r\n");
    // if (headerEnd == std::string::npos) {
    //     // Fallback: Se não achar separador, trata tudo como corpo (comportamento de script mal comportado)
    //     _body = rawOutput;
    //     return;
    // }

    // std::string headersPart = rawOutput.substr(0, headerEnd);
    // _body = rawOutput.substr(headerEnd + 4); // Pula o \r\n\r\n

    // // 2. Processar cabeçalhos linha a linha
    // std::stringstream ss(headersPart);
    // std::string line;
    
    // while (std::getline(ss, line)) {
    //     if (!line.empty() && line[line.size() - 1] == '\r') {
    //         line.erase(line.size() - 1); // Remove \r
    //     }
    //     if (line.empty()) continue;

    //     size_t colonPos = line.find(':');
    //     if (colonPos != std::string::npos) {
    //         std::string key = line.substr(0, colonPos);
    //         std::string value = line.substr(colonPos + 1);
            
    //         // Trim espaços
    //         size_t first = value.find_first_not_of(' ');
    //         if (first != std::string::npos)
    //             value = value.substr(first);

    //         // Tratamento especial para "Status"
    //         if (key == "Status") {
    //             // Exemplo: "Status: 404 Not Found"
    //             // O servidor deve usar esse status na linha de resposta HTTP
    //             size_t spacePos = value.find(' ');
    //             if (spacePos != std::string::npos) {
    //                 int statusCode = std::atoi(value.substr(0, spacePos).c_str());
    //                 setStatusCode(statusCode); // Método da sua classe Response
    //             }
    //         }
    //         else {
    //             // Adiciona aos headers da resposta
    //             setHeader(key, value);
    //         }
    //     }
    // }
    
    // Ajusta o Content-Length baseado no corpo real extraído
    // setHeader("Content-Length", ParseUtils::itoa(_body.size()));
	// }
