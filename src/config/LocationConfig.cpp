#include "config/LocationConfig.hpp"

LocationConfig::LocationConfig(std::string path) :
	BaseConfig(0),
	_path(path),
	_return(0, "")
{
	initDirectiveMap();
};

LocationConfig::LocationConfig(const LocationConfig& other) : 
	BaseConfig(other),
	_path(other._path),
	_methods(other._methods),
	_return(other._return)
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
	_directiveSetters["return"] = &LocationConfig::setReturn;
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

void LocationConfig::setReturn(const std::vector<std::string>&values)
{
	if (_return.first != 0)
		throw std::runtime_error("Duplicate 'return' directive in location.");
	if (values.empty())
		throw std::invalid_argument("return must have at least one value.");
	if (values.size() > 2)
		throw std::invalid_argument("return can't have more than 2 arguments.");

	if (!ParseUtils::isnumber(values[0]))
		throw std::invalid_argument("return code " + values[0] + " is not a number.");
	int return_code = std::atoi(values[0].c_str());
	if (return_code < 100 || return_code > 599)
		throw std::invalid_argument("invalid return code number (must be between 100 and 599).");
	
	std::string path = (values.size() == 2) ? values[1] : "";

	_return = std::make_pair(return_code, path);
}


const std::string& 					LocationConfig::getPath(void) const { return _path; };

const std::set<std::string>&		LocationConfig::getMethods(void) const { return _methods; };

const std::pair<int, std::string>&	LocationConfig::getReturn(void) const { return _return; };

bool	LocationConfig::hasReturn(void) const {
	return _return.first != 0;
}

bool	LocationConfig::isRedirectReturn(void) const {
	return _return.first >= 300 && _return.first < 400;
}

bool	LocationConfig::isErrorReturn(void) const {
	return _return.first >= 400 && _return.first < 600;
}

int		LocationConfig::getReturnCode(void) const {
	return _return.first;
}

const std::string&	LocationConfig::getReturnPath(void) const {
	return _return.second;
}
