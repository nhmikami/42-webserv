#ifndef SERVER_CONFIG_HPP
#define SERVER_CONFIG_HPP

#include "Request.hpp"
#include "Response.hpp"
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <map>
#include <vector>

class ServerConfig {
	private:
		std::string							_root;
		std::vector<std::string>			_index_files;
		std::map<std::string, std::string>	_error_pages;
		bool								_autoindex;

	public:
		ServerConfig(void) : _autoindex(true) {}
		~ServerConfig(void) {}

		std::string getRoot() const {
			return _root;
		}

		std::vector<std::string> getIndexFiles() const {
			return _index_files;
		}

		std::map<std::string, std::string> getErrorPages() const {
			return _error_pages;
		}

		bool getAutoindex() const {
			return _autoindex;
		}

		void handleRequest() {
			// parse request

			// get response

			// send response to client
		}
};

#endif