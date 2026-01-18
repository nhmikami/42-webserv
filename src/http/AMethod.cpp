#include "http/AMethod.hpp"
#include "utils/FileUtils.hpp"
#include "utils/Logger.hpp"

AMethod::AMethod(const Request& req, const ServerConfig& config, const LocationConfig* location)
	: _req(req), _config(config), _location(NULL), _cgiHandler(NULL) {
	if (location)
		_location = new LocationConfig(*location);
}

AMethod::~AMethod(void) {
	if (_cgiHandler) 
		delete _cgiHandler;
	if (_location)
		delete _location;
}

const Response& AMethod::getResponse(void) const {
	return _res;
}

const Request& AMethod::getRequest(void) const {
	return _req;
}

const ServerConfig& AMethod::getServerConfig(void) const {
	return _config;
}

const LocationConfig* AMethod::getLocationConfig(void) const {
	return _location;
}

CgiHandler* AMethod::getCgiHandler(void) const {
	return _cgiHandler;
}

CgiHandler* AMethod::releaseCgiHandler(void) {
	CgiHandler* temp = _cgiHandler;
	_cgiHandler = NULL;
	return temp;
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

size_t AMethod::_getMaxBodySize(void) const {
	if (_location) {
		size_t loc_size = _location->getClientMaxBodySize();
		if (loc_size > 0)
			return loc_size;
	}
	return _config.getClientMaxBodySize();
}

std::vector<std::string> AMethod::_getIndexFiles(void) const {
	if (_location) {
		std::vector<std::string> index_files = _location->getIndexFiles();
		if (!index_files.empty())
			return index_files;
	}
	return _config.getIndexFiles();
}

std::string AMethod::_getUploadLocation(void) {
	std::string uploadPath;
	if (_location && !_location->getUpload().empty())
		uploadPath = _location->getUpload();
	else
		uploadPath = _config.getUpload();

	if (uploadPath.empty())
		return ""; 
	if (uploadPath[0] == '/' || (uploadPath[0] == '.' && uploadPath[1] == '/')) 
		return uploadPath;
	return FileUtils::resolvePath(_getRootPath(), uploadPath);
}

bool AMethod::_isCGI(const std::string& path) const {
	size_t dotPos = path.find_last_of('.');
	if (dotPos == std::string::npos || dotPos == path.length() - 1)
		return false;

	std::string extension = path.substr(dotPos);
	if (_location) {
		const std::map<std::string, std::vector<std::string> >& locCgi = _location->getCgi();
		if (locCgi.count(extension) > 0)
			return true;
	}
	const std::map<std::string, std::vector<std::string> >& srvCgi = _config.getCgi();
	if (srvCgi.count(extension) > 0)
		return true;

	return false;
}

const std::map<std::string, std::vector<std::string> >& AMethod::_getCgiExecutors(void) const {
	if (_location && !_location->getCgi().empty())
		return _location->getCgi();

	return _config.getCgi();
}

HttpStatus AMethod::_runCGI(const std::string &path) {
	std::string ext = path.substr(path.find_last_of('.'));
	const std::map<std::string, std::vector<std::string> >& executors = _getCgiExecutors();
	if (!executors.count(ext))
		return FORBIDDEN;

	const std::vector<std::string>& executor = executors.at(ext);

	_cgiHandler = new CgiHandler(_req, path, executor);
	return CGI_PENDING;
}

std::string AMethod::_stripLocationPrefix(const std::string& path) const {
	if (!_location)
		return path;
	
	const std::string& prefix = _location->getPath();
	if (path.find(prefix) == 0)
		return path.substr(prefix.size());
	
	return path;
}
