#include "http/MethodGET.hpp"
#include "utils/FileUtils.hpp"
#include "utils/ParseUtils.hpp"

MethodGET::MethodGET(const Request& req, const ServerConfig& config, const LocationConfig* location)
	: AMethod(req, config, location) {}

MethodGET::~MethodGET(void) {}

HttpStatus MethodGET::handleMethod(void) {
	std::string full_path = FileUtils::resolvePath(_getRootPath(), _stripLocationPrefix(_req.getPath()));

	if (!FileUtils::exists(full_path))
		return NOT_FOUND;

	if (_isCGI(full_path) && FileUtils::isFile(full_path))
		return _runCGI(full_path);
	else if (FileUtils::isFile(full_path))
		return _serveFile(full_path);
	else if (FileUtils::isDirectory(full_path))
		return _serveDirectory(full_path);

	return FORBIDDEN;
}

HttpStatus MethodGET::_serveFile(const std::string& path) {
	if (!FileUtils::isReadable(path))
		return FORBIDDEN;

	struct stat file_stat;
	if (stat(path.c_str(), &file_stat) != 0 || file_stat.st_size < 0)
		return SERVER_ERR;
	size_t file_size = static_cast<size_t>(file_stat.st_size);
	if (file_size > _getMaxBodySize())
		return PAYLOAD_TOO_LARGE;
	
	std::ifstream file(path.c_str(), std::ios::binary);
	std::vector<char> buffer(file_size);
	file.read(buffer.data(), file_size);
	if (!file)
		return SERVER_ERR;

	_res.setBody(std::string(buffer.begin(), buffer.end()));
	_res.addHeader("Content-Type", FileUtils::guessMimeType(path));	
	return OK;
}

HttpStatus MethodGET::_serveDirectory(const std::string& path) {
	std::vector<std::string> index_files = _getIndexFiles();
	for (size_t i = 0; i < index_files.size(); i++) {
		std::string index_path;
		index_path = FileUtils::resolvePath(path, index_files[i]);

		if (FileUtils::exists(index_path) && FileUtils::isFile(index_path)) {
			if (_isCGI(index_path))
				return _runCGI(index_path);
			return _serveFile(index_path);
		}
	}
	if (_getAutoindex())
		return _generateAutoindex(path);
		
	return FORBIDDEN;
}

HttpStatus MethodGET::_generateAutoindex(const std::string& path) {
	DIR *dir;
	struct dirent *ent;
	std::vector<std::string> entries;

	dir = opendir(path.c_str());
	if (!dir)
		return FORBIDDEN;

	while ((ent = readdir(dir)) != NULL) {
		std::string entry = ent->d_name;
		if (entry == ".")
			continue ;
		entries.push_back(entry);
	}
	closedir(dir);
	
	std::stringstream html;
	html << "<html>\n<head><title>Index of " << ParseUtils::htmlEscape(_req.getPath()) << "</title></head>\n";
	html << "<body>\n<h1>Index of " << ParseUtils::htmlEscape(_req.getPath()) << "</h1>\n";
	html << "<ul>\n";
	
	std::sort(entries.begin(), entries.end());
	for (size_t i = 0; i < entries.size(); i++) {
		std::string entry = entries[i];
		if (entry == "..") {
			if (!_req.getPath().empty() && _req.getPath() != "/")
				html << "<li><a href=\"../\">../</a></li>\n";
			continue;
		}
		std::string full_path = FileUtils::resolvePath(path, entry);
		if (FileUtils::exists(full_path)) {
			std::string display_name = entry;
			std::string href = FileUtils::resolvePath(_req.getPath(), entry);
			if (FileUtils::isDirectory(full_path)) {
				display_name += "/";
				href += "/";
			}
			html << "<li><a href=\"" << ParseUtils::htmlEscape(href) << "\">" << ParseUtils::htmlEscape(display_name) << "</a></li>\n";
		}
	}
	html << "</ul>\n</body>\n</html>\n";
	_res.setBody(html.str());
	_res.addHeader("Content-Type", "text/html");
	return OK;
}
