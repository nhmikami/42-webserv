#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include LOCATION_HPP

struct ServerConfig {
	std::string							_host;
	int									_port
	std::string							_root;
	std::string							_server_name;
	bool								_autoindex = false;
	size_t								_client_max_body_size = DEFAULT;
	std::map<std::string, std::string>	_index_files;
	std::map<int, std::string>			_error_pages;
	std::vector<LocationConfig> 		_locations;
}

#endif