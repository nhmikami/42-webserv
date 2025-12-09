#ifndef METHOD_DELETE_HPP
#define METHOD_DELETE_HPP

#include "http/AMethod.hpp"
#include <cstring>
#include <dirent.h>
#include <unistd.h>

class MethodDELETE : public AMethod {
	private:
		bool	_canDelete(const std::string &path);
		bool	_isEmptyDirectory(const std::string &path);
		bool	_deleteFile(const std::string &path);
		bool	_deleteDirectory(const std::string &path);

	public:
		MethodDELETE(const Request &req, const ServerConfig &config, const LocationConfig* location);
		~MethodDELETE(void);

		HttpStatus handleMethod(void);
};

#endif