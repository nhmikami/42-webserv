#ifndef BASECONFIG_HPP
#define BASECONFIG_HPP

#include <string>
#include <vector>
#include <map>
#include <stdlib.h>
#include <sys/stat.h>

#include "utils/ParseUtils.hpp"

class BaseConfig {
	protected:
		std::string     					_root;
		bool            					_autoindex;
		size_t          					_client_max_body_size;
		std::vector<std::string>			_index_files;
		std::map<int, std::string>			_error_pages;
		bool								_is_cgi;
		std::map<std::string, std::string>	_cgi;
		std::string							_upload;

		void	setRoot(const std::vector<std::string>& values);
		void	setAutoIndex(const std::vector<std::string>& values);
		void	setClientMaxBodySize(const std::vector<std::string>& values);
		void	setIndexFiles(const std::vector<std::string>& values);
		void	setErrorPages(const std::vector<std::string>& values);
		void	setCgi(const std::vector<std::string>& values);
		void	setUpload(const std::vector<std::string>& values);

	public:
		BaseConfig(void);
		BaseConfig(const BaseConfig &other);
		virtual ~BaseConfig(void);

		const std::string 							getRoot(void) const;
		bool										getAutoIndex(void) const;
		size_t										getClientMaxBodySize(void) const;
		const std::vector<std::string>				getIndexFiles(void) const;
		const std::map<int, std::string>			getErrorPages(void) const;
		const std::map<std::string, std::string>	getCgi(void) const;
		const std::string							getUpload(void) const;
};

#endif