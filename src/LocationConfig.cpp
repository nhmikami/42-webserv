#include "LocationConfig.hpp"

// LocationConfig::LocationConfig(void) : 
// 	_autoindex(false),
// 	_client_max_body_size(1000),
// 	_is_cgi(false)
// {
//     initDirectiveMap();
// }

LocationConfig::LocationConfig(std::string path) : 
	_path(path),
	_autoindex(false),
	_client_max_body_size(1000),
	_is_cgi(false) 
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
	if (!ParseUtils::isnumber(values[0]))
		throw std::invalid_argument("client_max_body_size must be a number.");
	int client_max_body_size = std::atoi(values[0].c_str());
	if (client_max_body_size < 1)
		throw std::invalid_argument("client_max_body_size must be a positive number.");
	_client_max_body_size = client_max_body_size;
};

void LocationConfig::setMethods(const std::vector<std::string>& values)
{
	if (values.empty())
		throw std::invalid_argument("methods must have at least one value.");
	for (size_t i = 0; i < values.size(); i++)
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
	if (values.empty() || values.size() < 2)
		throw std::invalid_argument("error_page must have at least two values (error number and error file).");

	std::string path = values[values.size() - 1];
	for (size_t i = 0; i < values.size() - 1; i++) {
		if (!ParseUtils::isnumber(values[i]))
			throw std::invalid_argument("error code " + values[i] + " is not a number.");
		int error_code = std::atoi(values[i].c_str());
		if (error_code < 100 || error_code > 599)
			throw std::invalid_argument("invalid error code number (must be between 100 and 599).");
		_error_pages[error_code] = path;
	}
};

void LocationConfig::setCgi(const std::vector<std::string>& values)
{
	if (values.size() != 2)
		throw std::invalid_argument("cgi must have two values: extension and root.");
	_is_cgi = true;
	_cgi[values[0]] = values[1];
};



const std::string 							LocationConfig::getPath(void) const { return _path; };

const std::string 							LocationConfig::getRoot(void) const { return _root; };

const std::set<std::string>					LocationConfig::getMethods(void) const { return _methods; };

const std::vector<std::string>				LocationConfig::getIndexFiles(void) const { return _index_files; };

bool										LocationConfig::getAutoIndex(void) const { return _autoindex; };

size_t										LocationConfig::getClientMaxBodySize(void) const { return _client_max_body_size; };

const std::map<int, std::string>			LocationConfig::getErrorPages(void) const { return _error_pages; };

const std::map<std::string, std::string>	LocationConfig::getCgi(void) const { return _cgi; };