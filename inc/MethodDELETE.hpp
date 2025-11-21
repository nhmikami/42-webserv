#ifndef METHOD_DELETE_HPP
#define METHOD_DELETE_HPP

#include "AMethod.hpp"
#include <cstring>
#include <dirent.h>

class MethodDELETE : public AMethod {
	private:
	bool	_canDelete(const std::string &path);
	bool	_isEmptyDirectory(const std::string &path);
	bool	_deleteFile(const std::string &path);
	bool	_deleteDirectory(const std::string &path);
	HttpStatus	_runCGI(const std::string &path);

	public:
		MethodDELETE(const Request &req, const ServerConfig &config);
		~MethodDELETE(void) {}

		HttpStatus handleMethod(void);
};

MethodDELETE::MethodDELETE(const Request &req, const ServerConfig &config)
	: AMethod(req, config) {}


HttpStatus MethodDELETE::handleMethod(void) {
	std::string full_path = _joinPath(_config.getRoot(), _req.getPath());

	if (_isCGI(full_path))
		return _runCGI(full_path);

	if (!_exists(full_path))
		return NOT_FOUND;

	if (!_canDelete(full_path))
		return FORBIDDEN;

	if (_isFile(full_path)) {
		if (_deleteFile(full_path))
			return NO_CONTENT;
		return SERVER_ERR;
	}

	if (_isDirectory(full_path)) {
		if (!_isEmptyDirectory(full_path))
			return CONFLICT;

		if (_deleteDirectory(full_path))
			return NO_CONTENT;
		return SERVER_ERR;
	}
	return SERVER_ERR;
}

bool MethodDELETE::_canDelete(const std::string &path) {
	size_t pos = path.find_last_of('/');
	std::string parentDir;
	if (pos == 0)
		parentDir = "/";
	else
		parentDir = path.substr(0, pos);
	if (access(parentDir.c_str(), W_OK | X_OK) != 0)
		return false;

	if (_isDirectory(path)) {
		if (access(path.c_str(), W_OK | X_OK) != 0)
			return false;
	}
	return true;
}

bool MethodDELETE::_isEmptyDirectory(const std::string &path) {
	DIR *dir = opendir(path.c_str());
	if (!dir)
		return false;
	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
			closedir(dir);
			return false;
		}
	}
	closedir(dir);
	return true;
}

bool MethodDELETE::_deleteFile(const std::string &path) {
	return (unlink(path.c_str()) == 0);
}

bool MethodDELETE::_deleteDirectory(const std::string &path) {
	return (rmdir(path.c_str()) == 0);
}

HttpStatus MethodDELETE::_runCGI(const std::string &path) {
	// Implement CGI execution for DELETE method if needed
	return NOT_IMPLEMENTED;
}

#endif