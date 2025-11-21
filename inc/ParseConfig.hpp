#ifndef PARSECONFIG_HPP
#define PARSECONFIG_HPP

#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>

#include "Config.hpp"
#include "Logger.hpp"
#include "ParseUtils.hpp"

class ParseConfig {
	private:
		std::string	_filename;
		Config		_config;
		Logger		_logger;


		ParseConfig(void);
		ParseConfig(const ParseConfig &other);
		
		ParseConfig &operator=(const ParseConfig &other);

	public:
		ParseConfig(const std::string &filename);
		~ParseConfig(void);

		Config parse();
};

#endif