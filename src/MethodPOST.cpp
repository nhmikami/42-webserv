#include "MethodPOST.hpp"

MethodPOST::MethodPOST(const Request &req, const ServerConfig &config)
	: AMethod(req, config) {}

MethodPOST::~MethodPOST(void) {}

HttpStatus MethodPOST::handleMethod(void) {
	std::string full_path = _resolvePath(_config.getRoot(), _req.getPath()); // tratar múltiplos '/' e '..'

	if (_isCGI(full_path))
		return _runCGI(full_path);

	if (_exists(full_path)) {
		if (_isFile(full_path)) {
			if (!_isWritable(full_path))
				return FORBIDDEN;

			if (_writeToFile(full_path, _req.getBody())) {
				_res.setBody("File updated successfully");
				return OK;
			}
			return SERVER_ERR;
		}
		else if (_isDirectory(full_path)) {
			if (!_isWritable(full_path)) 
				return FORBIDDEN;

			std::string filename = _req.getHeader("X-Filename");
			if (filename.empty())
				return BAD_REQUEST;

			if (full_path[full_path.size() - 1] != '/') 
				full_path += "/";
			std::string full = full_path + filename;
			if (_writeToFile(full, _req.getBody())) {
				_res.addHeader("Location", _req.getPath() + "/" + filename);
				_res.setBody("File created successfully");
				return CREATED;
			}
		}
		return SERVER_ERR;
	}
	else {
		std::string parent = full_path.substr(0, full_path.find_last_of('/'));
		if (!_exists(parent) || !_isDirectory(parent))
			return NOT_FOUND;

		if (!_isWritable(parent))
			return FORBIDDEN;

		if (_writeToFile(full_path, _req.getBody())) {
			 _res.addHeader("Location", _req.getPath());
			 _res.setBody("File created successfully");
			return CREATED;
		}
		return SERVER_ERR;
	}
	return NOT_FOUND;
}

bool MethodPOST::_writeToFile(const std::string &path, const std::string &body) {
	int flags = O_WRONLY | O_CREAT | O_TRUNC;
	int fd = open(path.c_str(), flags, 0644);
	if (fd < 0)
		return false;

	size_t total_written = 0;
	size_t to_write = body.size();
	const char* buf = body.c_str();
	while (to_write > 0) {
		ssize_t written = write(fd, buf + total_written, to_write);
		if (written < 0) {
			close(fd);
			return false;
		}
		total_written += written;
		to_write -= written;
	}

	close(fd);
	return true;
}

HttpStatus MethodPOST::_runCGI(const std::string &path) {
	return NOT_IMPLEMENTED;
}
