#include "MethodPOST.hpp"

MethodPOST::MethodPOST(const Request &req, const ServerConfig &config)
	: AMethod(req, config) {}

MethodPOST::~MethodPOST(void) {}

HttpStatus MethodPOST::handleMethod(void) {
	if (_location) {
		std::set<std::string> allowed = _location->getMethods();
		if (!allowed.empty() && allowed.find("POST") == allowed.end()) {
			return NOT_ALLOWED;
		}
	}

	if (_req.getBody().size() > _getMaxBodySize())
		return PAYLOAD_TOO_LARGE; // 413

	std::string full_path = _resolvePath(_getRootPath(), _req.getPath());

	if (_exists(full_path)) {
		if (_isCGI(full_path))
			return _runCGI(full_path);
			
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

			std::string file_path = _resolvePath(full_path, filename);
			if (_writeToFile(file_path, _req.getBody())) {
				_res.addHeader("Location", _buildAbsoluteUrl(filename));
				_res.setBody("File created successfully");
				return CREATED;
			}
		}
	}
	else {
		std::string parent = full_path.substr(0, full_path.find_last_of('/'));
		if (!_exists(parent) || !_isDirectory(parent))
			return NOT_FOUND;

		if (!_isWritable(parent))
			return FORBIDDEN;

		if (_writeToFile(full_path, _req.getBody())) {
			 _res.addHeader("Location", _buildAbsoluteUrl(full_path));
			 _res.setBody("File created successfully");
			return CREATED;
		}
	}
	return SERVER_ERR;
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

std::string MethodPOST::_buildAbsoluteUrl(const std::string &targetPath) {
	std::string host = _req.getHeader("Host");
	if (host.empty())
		host = "localhost";

	return "http://" + host + targetPath;
}
