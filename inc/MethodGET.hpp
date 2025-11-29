#ifndef METHOD_GET_HPP
#define METHOD_GET_HPP

#include "AMethod.hpp"
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
		HttpStatus	_runCGI(const std::string &path);

	public:
		MethodGET(const Request &req, const ServerConfig &config);
		~MethodGET(void);
		
		HttpStatus	handleMethod(void);
};

#endif