#include "LocationConfig.hpp"

LocationConfig::LocationConfig(std::string path) : _path(path), _is_cgi(false) 
{
	initDirectiveMap();
};

LocationConfig::LocationConfig(const LocationConfig& other) : 
	_path(other._path),
	_root(other._root),
	_autoindex(other._autoindex),
	_client_max_body_size(other._client_max_body_size),
	_methods(other._methods),
	_index_files(other._index_files),
	_error_pages(other._error_pages),
	_is_cgi(other._is_cgi),
	_cgi(other._cgi)
{
	std::cout << "DEBUG: LocationConfig copy - path: " << _path 
				<< ", error_pages.size: " << _error_pages.size() 
				<< ", cgi.size: " << _cgi.size() << std::endl;
	initDirectiveMap();
}

LocationConfig::~LocationConfig() {};

void	LocationConfig::initDirectiveMap()
{
	_directiveSetters["root"] = &LocationConfig::setRoot;
	_directiveSetters["autoindex"] = &LocationConfig::setAutoIndex;
	_directiveSetters["client_max_body_size"] = &LocationConfig::setClientMaxBodySize;
	_directiveSetters["methods"] = &LocationConfig::setMethods;
	_directiveSetters["index"] = &LocationConfig::setIndexFiles;
	_directiveSetters["error_page"] = &LocationConfig::setErrorPages;
	_directiveSetters["cgi"] = &LocationConfig::setCgi;
}

void	LocationConfig::parseLocation(const std::string key, const std::vector<std::string> values)
{
	std::map<std::string, Setter>::iterator it = _directiveSetters.find(key);

	if (it != _directiveSetters.end())
		(this->*(it->second))(values);
	else
		throw std::invalid_argument("Unknown directive: " + key);
};

void LocationConfig::setRoot(const std::vector<std::string>& values)
{
	if (values.size() != 1)
		throw std::invalid_argument("root must have exactly one value.");
	_root = values[0];
};

void LocationConfig::setAutoIndex(const std::vector<std::string>& values)
{
	if (values.size() != 1)
		throw std::invalid_argument("autoindex must have exactly one value.");
	_autoindex = values[0] == "on" 
		? true 
		: values[0] == "off"
		? false
		: throw std::invalid_argument("autoindex invalid value.");
};

void LocationConfig::setClientMaxBodySize(const std::vector<std::string>& values)
{
	if (values.size() != 1)
		throw std::invalid_argument("client_max_body_size must have exactly one value.");
	_client_max_body_size = std::atoi(values[0].c_str());
};

void LocationConfig::setMethods(const std::vector<std::string>& values)
{
	if (values.empty())
		throw std::invalid_argument("methos must have at least one value.");
	for (size_t i = 0; i < values.size() - 1; i++)
		_methods.insert(values[i]);
};

void LocationConfig::setIndexFiles(const std::vector<std::string>& values)
{
	if (values.empty())
		throw std::invalid_argument("index must have at least one value.");
	_index_files = values;
};

void LocationConfig::setErrorPages(const std::vector<std::string>& values)
{
	if (values.empty())
		throw std::invalid_argument("error_page must have at least one value.");

	std::string path = values[values.size() - 1];
	for (size_t i = 0; i < values.size() - 1; i++)
		_error_pages[std::atoi(values[i].c_str())] = path;
};

void LocationConfig::setCgi(const std::vector<std::string>& values)
{
	std::cout << "CGI : " << values[0] << " : " << values[1] << std::endl;
	if (values.size() != 2)
		throw std::invalid_argument("cgi must have two values: extension and root.");
	_is_cgi = true;
	_cgi[values[0]] = values[1];
};



std::string 				LocationConfig::getPath(void) { return _path; };

std::string 				LocationConfig::getRoot(void) { return _root; };

std::set<std::string>		LocationConfig::getMethods(void) { return _methods; };

std::vector<std::string>	LocationConfig::getIndexFiles(void) { return _index_files; };

bool						LocationConfig::getAutoIndex(void) { return _autoindex; };

size_t						LocationConfig::getClientaMaxBodySize(void) { return _client_max_body_size; };

std::map<int, std::string>	LocationConfig::getErrorPages(void) { return _error_pages; };

std::map<std::string, std::string>	LocationConfig::getCgi(void) { 
	std::cout << "DEBUG getCgi: _cgi.size() = " << _cgi.size() << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = _cgi.begin(); it != _cgi.end(); ++it) {
        std::cout << "  " << it->first << " -> " << it->second << std::endl;
    }
	return _cgi; 
};