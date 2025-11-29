#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <set>
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <ostream>
#include <stdlib.h>

#include "ParseUtils.hpp"

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

		void	setPath(const std::vector<std::string>& values);
		void	setRoot(const std::vector<std::string>& values);
		void	setMethods(const std::vector<std::string>& values);
		void	setIndexFiles(const std::vector<std::string>& values);
		void	setAutoIndex(const std::vector<std::string>& values);
		void	setClientMaxBodySize(const std::vector<std::string>& values);
		void	setErrorPages(const std::vector<std::string>& values);
		void	setCgi(const std::vector<std::string>& values);

	public:
		LocationConfig(void);
		LocationConfig(std::string path);
		LocationConfig(const LocationConfig& other);
		~LocationConfig(void);

		void	parseLocation(const std::string key, const std::vector<std::string> values);
		void	initDirectiveMap();

		const std::string 							getPath(void) const;
		const std::string 							getRoot(void) const;
		bool										getAutoIndex(void) const;
		size_t										getClientMaxBodySize(void) const;
		const std::set<std::string>					getMethods(void) const;
		const std::vector<std::string>				getIndexFiles(void) const;
		const std::map<int, std::string>			getErrorPages(void) const;
		const std::map<std::string, std::string>	getCgi(void) const;
};

#endif