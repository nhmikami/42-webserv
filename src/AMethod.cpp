#include "AMethod.hpp"

AMethod::AMethod(const Request &req, const ServerConfig &config)
	: _req(req), _config(config) {}

AMethod::~AMethod(void) {}

Response AMethod::getResponse(void) const {
	return _res;
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
				return cleanRoot;
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

bool AMethod::_isCGI(const std::string& path) const {
	return false;
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
