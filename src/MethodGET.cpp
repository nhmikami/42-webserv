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
	if (!_isReadable(path))
		return FORBIDDEN;

	struct stat file_stat;
	if (stat(path.c_str(), &file_stat) != 0)
		return SERVER_ERR;
	size_t file_size = static_cast<size_t>(file_stat.st_size);
	if (file_size > _getMaxBodySize())
		return PAYLOAD_TOO_LARGE;
	
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
	if (_req.getPath()[_req.getPath().size() - 1] != '/') {
			std::string new_path = _req.getPath() + "/";
			_res.addHeader("Location", new_path);
			return MOVED_PERMANENTLY;
	}

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
	html << "<html>\n<head><title>Index of " << _htmlEscape(_req.getPath())<< "</title></head>\n";
	html << "<body>\n<h1>Index of " << _htmlEscape(_req.getPath()) << "</h1>\n";
	html << "<ul>\n";
	
	std::sort(entries.begin(), entries.end());
	for (size_t i = 0; i < entries.size(); i++) {
		std::string entry = entries[i];
		std::string full_path = _resolvePath(path, entry);

		if (_exists(full_path)) {
			std::string href = _resolvePath(_req.getPath(), entry);
			if (_isDirectory(full_path)) {
				entry += "/";
				href += "/";
			}
			html << "<li><a href=\"" << _htmlEscape(href) << "\">" << _htmlEscape(entry) << "</a></li>\n";
		}
	}
	html << "</ul>\n</body>\n</html>\n";
	_res.setBody(html.str());
	_res.addHeader("Content-Type", "text/html");
	return OK;
}

std::string MethodGET::_htmlEscape(const std::string &s) {
	std::string escaped;
	for (size_t i = 0; i < s.length(); ++i) {
		switch (s[i]) {
			case '&':
				escaped += "&amp;";
				break ;
			case '<':
				escaped += "&lt;";
				break ;
			case '>':
				escaped += "&gt;";
				break ;
			case '"':
				escaped += "&quot;";
				break ;
			case '\'':
				escaped += "&#39;";
				break ;
			default:
				escaped += s[i];
		}
	}
	return escaped;
}
