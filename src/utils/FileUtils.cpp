#include "utils/FileUtils.hpp"

bool FileUtils::exists(const std::string &path) {
	struct stat info;
	return (stat(path.c_str(), &info) == 0);
}

bool FileUtils::isDirectory(const std::string &path) {
	struct stat info;
	if (stat(path.c_str(), &info) == 0)
		return S_ISDIR(info.st_mode);

	return false;
}

bool FileUtils::isFile(const std::string &path) {
	struct stat info;
	if (stat(path.c_str(), &info) == 0)
		return S_ISREG(info.st_mode);

	return false;
}

bool FileUtils::isReadable(const std::string &path) {
	return (access(path.c_str(), R_OK) == 0);
}

bool FileUtils::isWritable(const std::string &path) {
	return (access(path.c_str(), W_OK) == 0);
}

bool FileUtils::isExecutable(const std::string &path) {
	return (access(path.c_str(), X_OK) == 0);
}

std::string FileUtils::resolvePath(const std::string &root, const std::string &reqPath) {
	std::string str1 = normalizePath(root);
	std::string str2 = normalizePath(reqPath);
	std::string final_path = str1 + '/' + str2;
	return final_path;

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
				continue ;
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

std::string FileUtils::normalizePath(const std::string& path) {
	std::stringstream ss(path);
	std::vector<std::string> tokens;
	std::string segment;
	while (std::getline(ss, segment, '/')) {
		if (segment.empty() || segment == ".")
			continue ;

		if (segment == "..") {
			if (tokens.empty())
				continue ;
			tokens.pop_back();
		} else {
			tokens.push_back(segment);
		}
	}

	std::string result;
	for (size_t i = 0; i < tokens.size(); ++i)
		result += "/" + tokens[i];
	if (result.empty())
		result = "/";

	return result;
}

std::string FileUtils::guessMimeType(const std::string &path) {
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
