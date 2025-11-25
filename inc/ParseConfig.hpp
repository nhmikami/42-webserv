#ifndef PARSECONFIG_HPP
#define PARSECONFIG_HPP

#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>

#include "Config.hpp"
#include "Logger.hpp"
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
		std::vector<ServerConfig> _servers;

		ParseConfig(void);
		ParseConfig(const ParseConfig &other);
		
		ParseConfig &operator=(const ParseConfig &other);

		bool changeContext(std::string line);
		void parseLine(std::string line);

	public:
		ParseConfig(const std::string &filename);
		~ParseConfig(void);

		std::vector<ServerConfig> parse();
};

#endif