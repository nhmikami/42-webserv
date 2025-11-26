#include "MethodGET.hpp"

MethodGET::MethodGET(const Request &req, const ServerConfig &config)
	: AMethod(req, config) {}

MethodGET::~MethodGET(void) {}

HttpStatus MethodGET::handleMethod(void) {
	std::string full_path = _resolvePath(_config.getRoot(), _req.getPath());

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
		return FORBIDDEN;

	std::stringstream buffer;
	buffer << file.rdbuf();
	_res.setBody(buffer.str());
	_res.addHeader("Content-Type", "text/html");
	return OK;
}

HttpStatus MethodGET::_serveDirectory(const std::string& fullpath) {
	std::vector<std::string> index_files = _config.getIndexFiles();
	if (index_files.empty()) 
        index_files.push_back("index.html");

    for (size_t i = 0; i < index_files.size(); i++) {
        std::string indexPath = fullpath;
        if (indexPath[indexPath.size() - 1] != '/')
            indexPath += "/";
        indexPath += index_files[i];

		if (_exists(indexPath) && _isFile(indexPath))
			return _serveFile(indexPath);
	}

	if (!_config.getAutoindex())
		return FORBIDDEN;

	return _generateAutoindex(fullpath);
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

		struct stat st;
		if (stat(full_path.c_str(), &st) == 0) {
			if (S_ISDIR(st.st_mode))
				entry += "/";

			html << "<li><a href=\"" << _req.getPath() << entry << "\">" << entry << "</a></li>\n";
		}
	}
	html << "</ul>\n</body>\n</html>\n";
	_res.setBody(html.str());
	_res.addHeader("Content-Type", "text/html");
	return OK;
}

HttpStatus MethodGET::_runCGI(const std::string &path) {
	return NOT_IMPLEMENTED;
}
