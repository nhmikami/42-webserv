#ifndef AMETHOD_HPP
#define AMETHOD_HPP

#include "Request.hpp"
#include "ServerConfig.hpp"
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

class AMethod {
	protected:
		Response			_res;
		const Request&		_req;
		const ServerConfig&	_config;

	public:
		AMethod(const Request &req, const ServerConfig &config);
		virtual ~AMethod(void);
		virtual HttpStatus	handleMethod(void) = 0;

		Response	getResponse(void) const;

	protected:
		bool	_exists(const std::string& path);
		bool	_isCGI(const std::string& path);
		bool	_isFile(const std::string& path);
		bool	_isDirectory(const std::string& path);
		bool	_isReadable(const std::string& path);
		bool	_isWritable(const std::string& path);
		bool	_isExecutable(const std::string& path);
		std::string	_resolvePath(const std::string &root, const std::string &reqPath);
};

#endif