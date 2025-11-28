#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <map>
#include <utility>
#include <vector>
#include <ostream>
#include <iostream>
#include <stdlib.h>

#include "LocationConfig.hpp"
#include "ParseUtils.hpp"

class ServerConfig {
	private:
		std::string								_host;
		int										_port;
		std::string								_root;
		std::string								_server_name;
		bool									_autoindex;
		size_t									_client_max_body_size;
		std::vector<std::string>				_index_files;
		std::map<int, std::string>				_error_pages;
		std::map<std::string, LocationConfig>	_locations;

		typedef void(ServerConfig::*Setter)(const std::vector<std::string>&);
		std::map<std::string, Setter>			_directiveSetters;

		void	initDirectiveMap();

		void	setListen(const std::vector<std::string>& values);
		void	setHost(const std::vector<std::string>& values);
		void	setRoot(const std::vector<std::string>& values);
		void	setServerName(const std::vector<std::string>& values);
		void	setAutoIndex(const std::vector<std::string>& values);
		void	setClientMaxBodySize(const std::vector<std::string>& values);
		void	setIndexFiles(const std::vector<std::string>& values);
		void	setErrorPages(const std::vector<std::string>& values);


	public:
		ServerConfig();
		~ServerConfig();

		void	parseServer(const std::string key, const std::vector<std::string> values);
		void	addLocation(const std::vector<std::string>&values, std::string *location_path);

		std::string								getHost(void);
		int										getPort(void);
		std::string								getRoot(void);
		std::string 							getServerName(void);
		bool									getAutoIndex(void);
		size_t									getClientaMaxBodySize(void);
		std::vector<std::string>				getIndexFiles(void);
		std::map<int, std::string>				getErrorPages(void);
		std::map<std::string, LocationConfig>	getLocations(void);
		LocationConfig*							getLocation(const std::string path);
};

#endif