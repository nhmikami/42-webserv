#ifndef METHOD_POST_HPP
#define METHOD_POST_HPP

#include "AMethod.hpp"
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

class MethodPOST : public AMethod {
	private:
		bool		_writeToFile(const std::string &path, const std::string &body);
		std::string	_buildAbsoluteUrl(const std::string &targetPath);

	public:
		MethodPOST(const Request &req, const ServerConfig &config);
		~MethodPOST(void);

		HttpStatus	handleMethod(void);
};

#endif