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

		const std::string							getHost(void) const;
		int											getPort(void) const;
		const std::string							getRoot(void) const;
		const std::string 							getServerName(void) const;
		bool										getAutoIndex(void) const;
		size_t										getClientMaxBodySize(void) const;
		const std::vector<std::string>				getIndexFiles(void) const;
		const std::map<int, std::string>			getErrorPages(void) const;
		const std::map<std::string, LocationConfig>	getLocations(void) const;
		LocationConfig*								getLocation(const std::string path);
};

#endif