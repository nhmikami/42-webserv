#include "MethodGET.hpp"

MethodGET::MethodGET(const Request &req, const ServerConfig &config)
	: AMethod(req, config) {}

MethodGET::~MethodGET(void) {}

HttpStatus MethodGET::handleMethod(void) {
	if (_location) {
		std::set<std::string> allowed = _location->getMethods();
		if (!allowed.empty() && allowed.find("GET") == allowed.end()) {
			return NOT_ALLOWED;
		}
	}

	std::string full_path = _resolvePath(_getRootPath(), _req.getPath());

	if (!_exists(full_path))
		return NOT_FOUND;

	if (_isCGI(full_path))
		return _runCGI(full_path);
	else if (_isFile(full_path))
		return _serveFile(full_path);
	else if (_isDirectory(full_path))
		return _serveDirectory(full_path);

	return FORBIDDEN;
}

HttpStatus MethodGET::_serveFile(const std::string& path) {
	if (!_isReadable(path)) {
		return FORBIDDEN;
	}

	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file)
		return SERVER_ERR;

	std::stringstream buffer;
	buffer << file.rdbuf();
	_res.setBody(buffer.str());
	_res.addHeader("Content-Type", _guessMimeType(path));
	return OK;
}

HttpStatus MethodGET::_serveDirectory(const std::string& path) {
	std::vector<std::string> index_files = _getIndexFiles();
	if (index_files.empty()) 
		index_files.push_back("index.html");

	for (size_t i = 0; i < index_files.size(); i++) {
		std::string index_path;
		index_path = _resolvePath(path, index_files[i]);

		if (_exists(index_path) && _isFile(index_path)) {
			if (_isCGI(index_path))
				return _runCGI(index_path);
			return _serveFile(index_path);
		}
	}

	if (_getAutoindex())
		return _generateAutoindex(path);
		
	return FORBIDDEN;
}

HttpStatus MethodGET::_generateAutoindex(const std::string &path) {
	DIR *dir;
	struct dirent *ent;
	std::vector<std::string> entries;

	dir = opendir(path.c_str());
	if (!dir)
		return FORBIDDEN;

	while ((ent = readdir(dir)) != NULL) {
		std::string entry = ent->d_name;
		if (entry == ".")
			continue;
		entries.push_back(entry);
	}
	closedir(dir);
	
	std::stringstream html;
	html << "<html>\n<head><title>Index of " << _req.getPath() << "</title></head>\n";
	html << "<body>\n<h1>Index of " << _req.getPath() << "</h1>\n";
	html << "<ul>\n";
	
	std::sort(entries.begin(), entries.end());
	for (size_t i = 0; i < entries.size(); i++) {
		std::string entry = entries[i];
		std::string full_path = _resolvePath(path, entry);

		if (_exists(full_path)) {
			if (_isDirectory(full_path))
				entry += "/";
			
			std::string href = _req.getPath();
			if (!href.empty() && href[href.size() - 1] != '/')
				href += "/";
			href += entry;
			html << "<li><a href=\"" << href << "\">" << entry << "</a></li>\n";
		}
	}
	html << "</ul>\n</body>\n</html>\n";
	_res.setBody(html.str());
	_res.addHeader("Content-Type", "text/html");
	return OK;
}

HttpStatus MethodGET::_runCGI(const std::string &path) {
	(void)path;
	return NOT_IMPLEMENTED;
}
