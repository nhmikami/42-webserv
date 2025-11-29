#include "ServerConfig.hpp"

ServerConfig::ServerConfig() :
	_host("127.0.0.1"),
	_port(80),
	_autoindex(false),
	_client_max_body_size(1000)
{
	initDirectiveMap();
};

ServerConfig::~ServerConfig() {};

void ServerConfig::addLocation(const std::vector<std::string>&values, std::string *location_path)
{
	if (values.size() != 2 || (values[0][0] != '/' || values[1] != "{"))
		throw std::invalid_argument("location must have a path and an open bracket at the same line.");
	_locations.insert(std::pair<std::string, LocationConfig>(values[0], LocationConfig(values[0])));
	*location_path = values[0];
};

void ServerConfig::initDirectiveMap()
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

void ServerConfig::parseServer(const std::string key, const std::vector<std::string> values)
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
	if (!ParseUtils::isnumber(values[0]))
		throw std::invalid_argument("listen must be a number.");
	int port = std::atoi(values[0].c_str());
	if (port < 1 || port > 65535)
		throw std::invalid_argument("port number must be between 1 and 65535.");
	_port = port;
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
	if (!ParseUtils::isnumber(values[0]))
		throw std::invalid_argument("client_max_body_size must be a number.");
	int client_max_body_size = std::atoi(values[0].c_str());
	if (client_max_body_size < 1)
		throw std::invalid_argument("client_max_body_size must be a positive number.");
	_client_max_body_size = client_max_body_size;
}

void ServerConfig::setIndexFiles(const std::vector<std::string>&values)
{
	if (values.empty())
		throw std::invalid_argument("index must have at least one value.");
	_index_files = values;
}

void ServerConfig::setErrorPages(const std::vector<std::string>& values)
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
}

const std::string							ServerConfig::getHost(void) const { return _host; };

int											ServerConfig::getPort(void) const { return _port; };

const std::string							ServerConfig::getRoot(void) const { return _root; };

const std::string							ServerConfig::getServerName(void) const { return _server_name; };

bool										ServerConfig::getAutoIndex(void) const { return _autoindex; };

size_t										ServerConfig::getClientMaxBodySize(void) const { return _client_max_body_size; };

const std::vector<std::string>				ServerConfig::getIndexFiles(void) const { return _index_files; };

const std::map<int, std::string>			ServerConfig::getErrorPages(void) const { return _error_pages; };

const std::map<std::string, LocationConfig>	ServerConfig::getLocations(void) const { return _locations; };

LocationConfig*								ServerConfig::getLocation(const std::string path) 
{
	std::map<std::string, LocationConfig>::iterator it = _locations.find(path);
    if (it != _locations.end())
        return &it->second;
    return NULL;
};


