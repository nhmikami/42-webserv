#include "AMethod.hpp"

AMethod::AMethod(const Request &req, const ServerConfig &config)
	: _req(req), _config(config), _location(NULL), _cgiHandler(NULL) {
	_location = _findLocation(_req.getPath());
}

AMethod::~AMethod(void) {
	if (_cgiHandler) 
		delete _cgiHandler;
}

const Response& AMethod::getResponse(void) const {
	return _res;
}

CgiHandler* AMethod::getCgiHandler(void) const {
	return _cgiHandler;
}

bool AMethod::_exists(const std::string &path) const {
	struct stat info;
	return (stat(path.c_str(), &info) == 0);
}

bool AMethod::_isDirectory(const std::string &path) const {
	struct stat info;

	if (stat(path.c_str(), &info) == 0)
		return S_ISDIR(info.st_mode);

	return false;
}

bool AMethod::_isFile(const std::string &path) const {
	struct stat info;

	if (stat(path.c_str(), &info) == 0)
		return S_ISREG(info.st_mode);

	return false;
}

bool AMethod::_isReadable(const std::string &path) const {
	return (access(path.c_str(), R_OK) == 0);
}

bool AMethod::_isWritable(const std::string &path) const {
	return (access(path.c_str(), W_OK) == 0);
}

bool AMethod::_isExecutable(const std::string &path) const {
	return (access(path.c_str(), X_OK) == 0);
}

std::string AMethod::_resolvePath(const std::string &root, const std::string &reqPath) {
	std::string cleanRoot = root;
	if (!cleanRoot.empty() && cleanRoot[cleanRoot.size() - 1] == '/')
		cleanRoot.erase(cleanRoot.size() - 1);

	std::stringstream ss(reqPath);
	std::vector<std::string> tokens;
	std::string segment;
	while (std::getline(ss, segment, '/')) {
		if (segment.empty() || segment == ".")
			continue;
		
		if (segment == "..") {
			if (tokens.empty())
				continue ; // return cleanRoot;
			tokens.pop_back();
		} 
		else
			tokens.push_back(segment);
	}

	std::string finalPath = cleanRoot;
	for (size_t i = 0; i < tokens.size(); ++i)
		finalPath += "/" + tokens[i];

	return finalPath;
}

const std::string AMethod::_guessMimeType(const std::string &path) const {
	size_t dot = path.find_last_of('.');
	if (dot == std::string::npos)
		return "application/octet-stream";

	std::string ext = path.substr(dot + 1);

	if (ext == "html" || ext == "htm") return "text/html";
	if (ext == "txt") return "text/plain";
	if (ext == "css") return "text/css";
	if (ext == "pdf") return "application/pdf";
	if (ext == "json") return "application/json";
	if (ext == "js") return "application/javascript";
	if (ext == "png") return "image/png";
	if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
	if (ext == "gif") return "image/gif";
	if (ext == "svg") return "image/svg+xml";
	if (ext == "ico") return "image/x-icon";

	return "application/octet-stream";
}

const LocationConfig* AMethod::_findLocation(const std::string& path) {
	const std::map<std::string, LocationConfig> &locations = _config.getLocations();
	const LocationConfig* bestMatch = NULL;
	std::string bestKey = "";

	for (std::map<std::string, LocationConfig>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
		const std::string &key = it->first;

		if (key.size() > path.size())
			continue;

		if (path.compare(0, key.size(), key) == 0) {
			if (key.size() == path.size() || path[key.size()] == '/' || key == "/") {
				if (key.size() > bestKey.size()) {
					bestMatch = &(it->second);
					bestKey = key;
				}
			}
		}
	}
	return bestMatch;
}

std::string AMethod::_getRootPath(void) const {
	if (_location && !_location->getRoot().empty())
		return _location->getRoot();
	return _config.getRoot();
}

bool AMethod::_getAutoindex(void) const {
	if (_location)
		return _location->getAutoIndex();
	return _config.getAutoIndex();
}

std::vector<std::string> AMethod::_getIndexFiles(void) const {
	if (_location) {
		std::vector<std::string> index_files = _location->getIndexFiles();
		if (!index_files.empty())
			return index_files;
	}
	return _config.getIndexFiles();
}

size_t AMethod::_getMaxBodySize(void) const {
	if (_location) {
		size_t loc_size = _location->getClientMaxBodySize();
		if (loc_size > 0)
			return loc_size;
	}
	return _config.getClientMaxBodySize();
}

std::string AMethod::_getUploadLocation(void) {
	std::string uploadPath;
	if (_location && !_location->getUpload().empty())
		uploadPath = _location->getUpload();
	else
		uploadPath = _config.getUpload();

	if (uploadPath.empty())
		return ""; 
	if (uploadPath[0] == '/')
		return uploadPath;
	return _resolvePath(_getRootPath(), uploadPath);
}

bool AMethod::_isCGI(const std::string& path) const {
	size_t dotPos = path.find_last_of(".");
	if (dotPos == std::string::npos || dotPos == path.length() - 1)
		return false;
	std::string extension = path.substr(dotPos);
	
	if (_location) {
		const std::map<std::string, std::string>& locCgi = _location->getCgi();
		if (locCgi.count(extension) > 0)
			return true;
	}

	const std::map<std::string, std::string>& srvCgi = _config.getCgi();
	if (srvCgi.count(extension) > 0)
		return true;

	return false;
}

std::map<std::string, std::string> AMethod::_getCgiExecutors(void) const {
	std::map<std::string, std::string> cgiMap;

	if (_location) {
		const std::map<std::string, std::string>& locCgi = _location->getCgi();
		cgiMap.insert(locCgi.begin(), locCgi.end());
	}

	const std::map<std::string, std::string>& srvCgi = _config.getCgi();
	cgiMap.insert(srvCgi.begin(), srvCgi.end());

	return cgiMap;
}

HttpStatus AMethod::_runCGI(const std::string &path) {
	std::map<std::string, std::string> executors = _getCgiExecutors();
	std::string	ext = path.substr(path.find_last_of('.'));
	std::string	executor = executors[ext];

	_cgiHandler = new CgiHandler(_req, path, executor);
	_cgiHandler->start(); 

	return CGI_PENDING;
}


// void Client::processRequest() {
	
// 	// ... parse request ...

// 	LocationConfig* loc = _server->findLocationForRequest(_request);
// 	// check if method is allowed in this location
// 	if (loc && !loc->isMethodAllowed(req.getMethodStr()))
// 		return makeErrorResponse(NOT_ALLOWED);

// 	// Cria o método (GET/POST)
// 	AMethod* method = NULL;
// 	if (req.getMethodStr() == "GET")
// 		method = new MethodGET(req, config, loc);
// 	else if (req.getMethodStr() == "POST")
// 		method = new MethodPOST(req, config, loc);
// 	else if (req.getMethodStr() == "DELETE")
// 		method = new MethodDELETE(req, config, loc);
// 	else
// 		return makeErrorResponse(NOT_ALLOWED);

// 	HttpStatus status = method->handleMethod();

// 	if (status == CGI_PENDING) {
// 		// CASO ESPECIAL: CGI Assíncrono
// 		// Não deletamos o handler ainda, pois precisamos dele para o Pipe
// 		_cgiHandler = method->getCgiHandler(); // Salva ponteiro
// 		_methodToCheckCgi = method;            // Salva para deletar depois

// 		// Configura o poller para ouvir o Pipe do CGI
// 		_server->addToPoller(method->getCgiOutputFd(), POLLIN, this);
// 		_state = CLIENT_WAITING_CGI;
// 	} 
// 	else {
// 		// CASO NORMAL (Arquivos estáticos, erros, etc)
// 		// Pega a resposta gerada pelo método
// 		_res = methodHandler->getResponse();
		
// 		// Se o método retornou um código de erro (ex: 404, 500) que não gerou body,
// 		// aqui seria o lugar de gerar a página de erro padrão (Error Pages).
// 		if (_res.getBody().empty() && status >= 400) {
// 			_generateErrorPage(status);
// 		} else {
// 			// Se o método retornou status diferente do que está no _res
// 			_res.setStatus(status); 
// 		}

// 		// Finaliza
// 		_res.buildResponse(); // Serializa headers+body
// 		_state = CLIENT_SENDING;
		
// 		// Limpeza
// 		delete method;
// 	}
	
// 	// Se não for CGI, finaliza resposta normal...
// }
