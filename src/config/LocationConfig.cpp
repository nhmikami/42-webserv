#include "config/LocationConfig.hpp"

LocationConfig::LocationConfig(std::string path) :
	BaseConfig(0),
	_path(path)
{
	initDirectiveMap();
};

LocationConfig::LocationConfig(const LocationConfig& other) : 
	BaseConfig(other),
	_path(other._path),
	_methods(other._methods)
{
	initDirectiveMap();
}

LocationConfig::~LocationConfig(void) {};

void	LocationConfig::initDirectiveMap()
{
	_directiveSetters["root"] = &LocationConfig::setRoot;
	_directiveSetters["autoindex"] = &LocationConfig::setAutoIndex;
	_directiveSetters["client_max_body_size"] = &LocationConfig::setClientMaxBodySize;
	_directiveSetters["methods"] = &LocationConfig::setMethods;
	_directiveSetters["index"] = &LocationConfig::setIndexFiles;
	_directiveSetters["error_page"] = &LocationConfig::setErrorPages;
	_directiveSetters["cgi"] = &LocationConfig::setCgi;
	_directiveSetters["upload"] = &LocationConfig::setUpload;
}

void	LocationConfig::parseLocation(const std::string key, const std::vector<std::string> values)
{
	std::map<std::string, Setter>::iterator it = _directiveSetters.find(key);

	if (it != _directiveSetters.end())
		(this->*(it->second))(values);
	else
		throw std::invalid_argument("Unknown directive: " + key);
};

void LocationConfig::setMethods(const std::vector<std::string>& values)
{
	if (values.empty())
		throw std::invalid_argument("methods must have at least one value.");
	for (size_t i = 0; i < values.size(); i++)
		_methods.insert(values[i]);
};


const std::string 				LocationConfig::getPath(void) const { return _path; };

const std::set<std::string>		LocationConfig::getMethods(void) const { return _methods; };
