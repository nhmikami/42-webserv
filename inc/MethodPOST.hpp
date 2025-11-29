#ifndef METHOD_POST_HPP
#define METHOD_POST_HPP

#include "AMethod.hpp"
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

class MethodPOST : public AMethod {
	private:
		bool		_writeToFile(const std::string &path, const std::string &body);
		HttpStatus	_runCGI(const std::string &path);

	public:
		MethodPOST(const Request &req, const ServerConfig &config);
		~MethodPOST(void);

		HttpStatus	handleMethod(void);
};

#endif