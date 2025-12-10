#ifndef AMETHOD_HPP
#define AMETHOD_HPP

#include "http/CgiHandler.hpp"
#include "http/Response.hpp"
#include "http/Request.hpp"
#include "config/LocationConfig.hpp"
#include "config/ServerConfig.hpp"
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

class AMethod {
	protected:
		Response				_res;
		const Request&			_req;
		const ServerConfig&		_config;
		LocationConfig*			_location;
		CgiHandler*				_cgiHandler;

	public:
		AMethod(const Request& req, const ServerConfig& config, const LocationConfig* location);
		virtual ~AMethod(void);
		virtual HttpStatus	handleMethod(void) = 0;

		const Response&			getResponse(void) const;
		const Request&			getRequest(void) const;
		const ServerConfig&		getServerConfig(void) const;
		const LocationConfig*	getLocationConfig(void) const;
		CgiHandler*				getCgiHandler(void) const;
		CgiHandler*				releaseCgiHandler(void);

	protected:
		std::string 		_stripLocationPrefix(const std::string& path) const;
		
		std::string					_getRootPath(void) const;
		bool						_getAutoindex(void) const;
		size_t						_getMaxBodySize(void) const;
		std::vector<std::string>	_getIndexFiles(void) const;
		std::string					_getUploadLocation(void);
		
		bool								_isCGI(const std::string& path) const;
		std::map<std::string, std::string>	_getCgiExecutors(void) const;
		HttpStatus							_runCGI(const std::string &path);

};

#endif