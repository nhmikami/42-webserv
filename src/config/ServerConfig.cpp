#include "config/ServerConfig.hpp"

ServerConfig::ServerConfig(void) :
	BaseConfig(),
	_host("127.0.0.1"),
	_host_set(false),
	_port(80),
	_port_set(false),
	_server_name("")
{
	initDirectiveMap();
};

ServerConfig::~ServerConfig(void) {};

void ServerConfig::addLocation(const std::vector<std::string>&values, std::string *location_path)
{
	if (values.size() != 2 || (values[0][0] != '/' || values[1] != "{"))
		throw std::invalid_argument("location must have a valid path and an open bracket at the same line.");
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
	_directiveSetters["cgi"] = &ServerConfig::setCgi;
	_directiveSetters["upload"] = &ServerConfig::setUpload;
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
	if (_port_set)
		throw std::invalid_argument("Duplicate 'listen' directive on server.");
	if (values.size() != 1)
		throw std::invalid_argument("listen must have exactly one value.");
	if (!ParseUtils::isUnsigNumber(values[0]))
		throw std::invalid_argument("listen must be a valid port number.");
	int port = std::atoi(values[0].c_str());
	if (port < 1 || port > 65535)
		throw std::invalid_argument("port number must be between 1 and 65535.");
	_port = port;
	_port_set = true;
}

bool ServerConfig::isValidIP(const std::string &ip)
{
	std::vector<std::string> octets = ParseUtils::split(ip, '.');
	if (octets.size() != 4)
		return false;

	for (size_t i = 0; i < octets.size(); i++)
	{
		if (octets[i].empty() || !ParseUtils::isUnsigNumber(octets[i]))
			return false;
		int num = std::atoi(octets[i].c_str());
		if (num < 0 || num > 255)
			return false;
	}

	return true;
}

bool ServerConfig::isValidDomain(const std::string &domain)
{
	if (domain.empty() || 
		domain.length() > 255 || 
		domain.find('.') == std::string::npos || 
		domain.find("..") != std::string::npos ||
		ParseUtils::isNumber(domain))
		return false;

	if (ParseUtils::hasSpecialChar(domain))
		return false;

	if (domain[0] == '.' || 
		domain[0] == '-' || 
		domain[domain.length() - 1] == '.' || 
		domain[domain.length() - 1] == '-')
		return false;

	return true;
}

void ServerConfig::setHost(const std::vector<std::string>&values)
{
	if (_host_set)
		throw std::invalid_argument("Duplicate 'host' directive on server.");
	if (values.size() != 1)
		throw std::invalid_argument("host must have exactly one value.");
	if (!isValidIP(values[0]) && !isValidDomain(values[0]))
		throw std::invalid_argument("host must be a valid IP address or domain name.");

	_host = values[0];
	_host_set = true;
}

void ServerConfig::setServerName(const std::vector<std::string>&values)
{
	if (_server_name != "")
		throw std::invalid_argument("Duplicate 'server_name' directive on server.");
	if (values.size() != 1)
		throw std::invalid_argument("server_name must have exactly one value.");
	if (ParseUtils::hasSpecialChar(values[0]))
		throw std::invalid_argument("server_name cannot have a special character.");
	_server_name = values[0];
}

const std::string							ServerConfig::getHost(void) const { return _host; };

int											ServerConfig::getPort(void) const { return _port; };

const std::string							ServerConfig::getServerName(void) const { return _server_name; };

const std::map<std::string, LocationConfig>	ServerConfig::getLocations(void) const { return _locations; };

LocationConfig*								ServerConfig::getLocation(const std::string path) 
{
	std::map<std::string, LocationConfig>::iterator it = _locations.find(path);
	if (it != _locations.end())
		return &it->second;
	return NULL;
};


