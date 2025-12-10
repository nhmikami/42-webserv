#ifndef METHOD_GET_HPP
#define METHOD_GET_HPP

#include "http/AMethod.hpp"
#include <algorithm>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <vector>

class MethodGET : public AMethod {
	private:
		HttpStatus	_serveFile(const std::string &path);
		HttpStatus	_serveDirectory(const std::string &path);
		HttpStatus	_generateAutoindex(const std::string &path);

	public:
		MethodGET(const Request& req, const ServerConfig& config, const LocationConfig* location);
		~MethodGET(void);
		
		HttpStatus	handleMethod(void);
};

#endif