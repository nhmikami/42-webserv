#ifndef METHOD_POST_HPP
#define METHOD_POST_HPP

#include "http/AMethod.hpp"
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <map>

class MethodPOST : public AMethod {
	private:
		std::map<std::string, std::string>	_formFields;
		
		bool		_writeToFile(const std::string& path, const std::string& body);
		bool		_writeToFile(const std::string& path, const char* buffer, size_t size);
		std::string	_buildAbsoluteUrl(const std::string& targetPath);
		std::string	_extractFilename(const std::string& filename);
		HttpStatus	_handleMultipart(void);
		HttpStatus	_runCGIWithFormFields(const std::string& path);

	public:
		MethodPOST(const Request& req, const ServerConfig& config, const LocationConfig* location);
		~MethodPOST(void);

		HttpStatus	handleMethod(void);
		const std::map<std::string, std::string>&	getFormFields(void) const;
};

#endif