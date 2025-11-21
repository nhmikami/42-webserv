#ifndef AMETHOD_HPP
#define AMETHOD_HPP

#include "Request.hpp"
#include "ServerConfig.hpp"

class AMethod {
	protected:
		Response			_res;
		const Request&		_req;
		const ServerConfig&	_config;

	public:
		AMethod(const Request &req, const ServerConfig &config);
		virtual ~AMethod(void);
		virtual HttpStatus	handleMethod(void) = 0;

		Response	getResponse(void) const;

	protected:
		bool	_exists(const std::string& path);
		bool	_isCGI(const std::string& path);
		bool	_isFile(const std::string& path);
		bool	_isDirectory(const std::string& path);
		bool	_isReadable(const std::string& path);
		bool	_isWritable(const std::string& path);
		bool	_isExecutable(const std::string& path);
		std::string	_joinPath(const std::string &root, const std::string &path);
};

AMethod::AMethod(const Request &req, const ServerConfig &config)
	: _req(req), _config(config) {}

AMethod::~AMethod(void) {}

Response AMethod::getResponse(void) const {
	return _res;
}

bool AMethod::_exists(const std::string &path) {
	struct stat info;
	return (stat(path.c_str(), &info) == 0);
}

bool AMethod::_isDirectory(const std::string &path) {
	struct stat info;

	if (stat(path.c_str(), &info) == 0)
		return S_ISDIR(info.st_mode);

	return false;
}

bool AMethod::_isFile(const std::string &path) {
	struct stat info;

	if (stat(path.c_str(), &info) == 0)
		return S_ISREG(info.st_mode);

	return false;
}

bool AMethod::_isReadable(const std::string &path) {
	return (access(path.c_str(), R_OK) == 0);
}

bool AMethod::_isWritable(const std::string &path) {
	return (access(path.c_str(), W_OK) == 0);
}

bool AMethod::_isExecutable(const std::string &path) {
	return (access(path.c_str(), X_OK) == 0);
}

std::string AMethod::_joinPath(const std::string &root, const std::string &path) {
	if (root.size() > 0 && root[root.size() - 1] != '/')
		return root + '/' + path;
	return root + path;
}


std::map<std::string, std::string> makeMimeMap(void) {
	std::map<std::string, std::string> mime_map;

	mime_map[".txt"] = "text/plain";
	mime_map[".html"] = "text/html";
	mime_map[".htm"] = "text/html";
	mime_map[".css"] = "text/css";
	mime_map[".jpg"] = "image/jpeg";
	mime_map[".jpeg"] = "image/jpeg";
	mime_map[".png"] = "image/png";
	mime_map[".gif"] = "image/gif";
	mime_map[".ico"] = "image/x-icon";
	mime_map[".pdf"] = "application/pdf";
	mime_map[".js"] = "application/javascript";
	mime_map[".json"] = "application/json";
	// Add more MIME types as needed

	return mime_map;
}

std::string getMimeType(const std::string &path) {
	static std::map<std::string, std::string> mime_map = makeMimeMap();

	size_t dotPos = path.rfind('.');
	if (dotPos == std::string::npos)
		return "application/octet-stream";

	std::string ext = path.substr(dotPos);
	std::map<std::string, std::string>::const_iterator it = mime_map.find(ext);
	if (it != mime_map.end())
		return it->second;

	return "application/octet-stream"; // default MIME type
}


#endif