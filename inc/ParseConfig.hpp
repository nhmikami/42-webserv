#ifndef PARSECONFIG_HPP
#define PARSECONFIG_HPP

#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <algorithm>

#include "ParseUtils.hpp"
#include "ServerConfig.hpp"

class ParseConfig {
	private:
		enum Context {
			GLOBAL,
			SERVER,
			LOCATION
		};

		std::string	_filename;
		Context		_context;
		int			_open_brackets;
		int			_count_line;
		std::string	_location_path;
		std::vector<ServerConfig> _servers;

		ParseConfig(void);
		ParseConfig(const ParseConfig &other);
		
		ParseConfig &operator=(const ParseConfig &other);

		bool getKeyValues(const std::string line, std::string *key, std::vector<std::string> *values);
		bool changeContext(const std::string key, std::vector<std::string> values);
		void parseLine(const std::string key, std::vector<std::string> values);

	public:
		ParseConfig(const std::string &filename);
		~ParseConfig(void);

		std::vector<ServerConfig> parse();
};

#endif