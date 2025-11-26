#include "ServerConfig.hpp"

ServerConfig::ServerConfig() :
	_host("127.0.0.1"),
	_port(80)
{
	initDirectiveMap();
};

ServerConfig::~ServerConfig() {};

void	ServerConfig::addLocation(const std::vector<std::string>&values)
{
	if (values.size() != 2 || (values[0][0] != '/' || values[1] != "{"))
		throw std::invalid_argument("location must have a path and an open bracket at the same line.");
	_locations.push_back(LocationConfig(values[0]));
};

void	ServerConfig::initDirectiveMap()
{
    _directiveSetters["listen"] = &ServerConfig::setListen;
    _directiveSetters["host"] = &ServerConfig::setHost;
    _directiveSetters["root"] = &ServerConfig::setRoot;
    _directiveSetters["server_name"] = &ServerConfig::setServerName;
    _directiveSetters["autoindex"] = &ServerConfig::setAutoIndex;
    _directiveSetters["client_max_body_size"] = &ServerConfig::setClientMaxBodySize;
    _directiveSetters["index"] = &ServerConfig::setIndexFiles;
    _directiveSetters["error_page"] = &ServerConfig::setErrorPages;
}

void	ServerConfig::parseServer(const std::string key, const std::vector<std::string> values)
{
	std::map<std::string, Setter>::iterator it = _directiveSetters.find(key);

	if (it != _directiveSetters.end())
		(this->*(it->second))(values);
	else
		throw std::invalid_argument("Unknown directive: " + key);
};

void ServerConfig::setListen(const std::vector<std::string>&values)
{
	if (values.size() != 1)
		throw std::invalid_argument("listen must have exactly one value.");
	_port = std::atoi(values[0].c_str());
}

void ServerConfig::setHost(const std::vector<std::string>&values)
{
	if (values.size() != 1)
		throw std::invalid_argument("host must have exactly one value.");
	_host = values[0];
}

void ServerConfig::setRoot(const std::vector<std::string>&values)
{
	if (values.size() != 1)
		throw std::invalid_argument("root must have exactly one value.");
	_root = values[0];
}

void ServerConfig::setServerName(const std::vector<std::string>&values)
{
	if (values.size() != 1)
		throw std::invalid_argument("server_name must have exactly one value.");
	_server_name = values[0];
}

void ServerConfig::setAutoIndex(const std::vector<std::string>&values)
{
	if (values.size() != 1)
		throw std::invalid_argument("autoindex must have exactly one value.");
	_autoindex = values[0] == "on" 
		? true 
		: values[0] == "off"
		? false
		: throw std::invalid_argument("autoindex invalid value.");
}

void ServerConfig::setClientMaxBodySize(const std::vector<std::string>& values)
{
	if (values.size() != 1)
		throw std::invalid_argument("client_max_body_size must have exactly one value.");
	_client_max_body_size = std::atoi(values[0].c_str());
}

void ServerConfig::setIndexFiles(const std::vector<std::string>&values)
{
	if (values.empty())
		throw std::invalid_argument("index must have at least one value.");
	_index_files = values;
}

void ServerConfig::setErrorPages(const std::vector<std::string>& values)
{
	if (values.empty())
		throw std::invalid_argument("error_page must have at least one value.");

	std::string path = values[values.size() - 1];
	for (size_t i = 0; i < values.size() - 1; i++)
		_error_pages[std::atoi(values[i].c_str())] = path;
}

std::string	ServerConfig::getHost(void) { return _host; };

int			ServerConfig::getPort(void) { return _port; };

std::string	ServerConfig::getRoot(void) { return _root; };

std::string ServerConfig::getServerName(void) { return _server_name; };

bool		ServerConfig::getAutoIndex(void) { return _autoindex; };

size_t		ServerConfig::getClientaMaxBodySize(void) { return _client_max_body_size; };

std::vector<std::string>	ServerConfig::getIndexFiles(void) { return _index_files; };

std::map<int, std::string>	ServerConfig::getErrorPages(void) { return _error_pages; };

std::vector<LocationConfig> &ServerConfig::getLocations(void) { return _locations; };

LocationConfig				ServerConfig::getCurrentLocation(void) { return _locations.back(); };


