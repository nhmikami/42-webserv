#ifndef METHOD_POST_HPP
#define METHOD_POST_HPP

#include "AMethod.hpp"
#include <fcntl.h>

class MethodPOST : public AMethod {
	private:
		bool	_writeToFile(const std::string &path, const std::string &body);
		HttpStatus	_runCGI(const std::string &path);

	public:
		MethodPOST(const Request &req, const ServerConfig &config);
		~MethodPOST(void) {}

		HttpStatus	handleMethod(void);
};

MethodPOST::MethodPOST(const Request &req, const ServerConfig &config)
	: AMethod(req, config) {}

HttpStatus MethodPOST::handleMethod(void) {
	std::string full_path = _joinPath(_config.getRoot(), _req.getPath()); // tratar múltiplos '/' e '..'

	if (_isCGI(full_path)) {
		HttpStatus status = OK;
		// status = _runCGI(path);
		return status;
	}

	if (_isFile(full_path)) {
		if (_exists(full_path) && !_isWritable(full_path))
			return FORBIDDEN;

		bool fileExists = _exists(full_path);
		if (_writeToFile(full_path, _req.getBody())) {
			if (fileExists)
				return OK;
			return CREATED;
		}
		return SERVER_ERR;
	}

	if (_isDirectory(full_path)) {
		if (!_isWritable(full_path))
			return FORBIDDEN;

		// Tentar pegar nome do arquivo enviado via header ou query
		std::string filename = _req.getHeader("X-Filename");
		if (filename.empty())
			return BAD_REQUEST;

		std::string full = full_path + "/" + filename;
		if (_writeToFile(full, _req.getBody()))
			return CREATED;
		return SERVER_ERR;
	}
	return NOT_FOUND;
}

bool MethodPOST::_writeToFile(const std::string &path, const std::string &body) {
	int flags = O_WRONLY | O_CREAT | O_TRUNC;
	int fd = open(path.c_str(), flags, 0644);
	if (fd < 0) {
		std::cerr << "Error: cannot open file " << path << std::endl;
		return false;
	}

	size_t total_written = 0;
	size_t to_write = body.size();
	const char* buf = body.c_str();
	while (to_write > 0) {
		ssize_t written = write(fd, buf + total_written, to_write);
		if (written < 0) {
			std::cerr << "Error: failed to write to " << path << std::endl;
			close(fd);
			return false;
		}
		total_written += written;
		to_write -= written;
	}

	close(fd);
	return true;
}



#endif