#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <set>
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <ostream>
#include <stdlib.h>

class LocationConfig {
	private:
		std::string							_path;
		std::string							_root;
		bool								_autoindex;
		size_t								_client_max_body_size;
		std::set<std::string>				_methods;
		std::vector<std::string>			_index_files;
		std::map<int, std::string>			_error_pages;
		bool 								_is_cgi;
		std::map<std::string, std::string>	_cgi;

		typedef void(LocationConfig::*Setter)(const std::vector<std::string>&);
		std::map<std::string, Setter>		_directiveSetters;

		void setPath(const std::vector<std::string>& values);
		void setRoot(const std::vector<std::string>& values);
		void setMethods(const std::vector<std::string>& values);
		void setIndexFiles(const std::vector<std::string>& values);
		void setAutoIndex(const std::vector<std::string>& values);
		void setClientMaxBodySize(const std::vector<std::string>& values);
		void setErrorPages(const std::vector<std::string>& values);
		void setCgi(const std::vector<std::string>& values);

	public:
		LocationConfig(std::string path);
		LocationConfig(const LocationConfig& other);
		~LocationConfig(void);

		void	parseLocation(const std::string key, const std::vector<std::string> values);
		void	initDirectiveMap();

		std::string 						getPath(void);
		std::string 						getRoot(void);
		bool								getAutoIndex(void);
		size_t								getClientaMaxBodySize(void);
		std::set<std::string>				getMethods(void);
		std::vector<std::string>			getIndexFiles(void);
		std::map<int, std::string>			getErrorPages(void);
		std::map<std::string, std::string>	getCgi(void);
};

#endif