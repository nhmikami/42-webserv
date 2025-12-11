#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <set>

#include "BaseConfig.hpp"

class LocationConfig : public BaseConfig {
	private:
		std::string						_path;
		std::set<std::string>			_methods;
		std::pair<int, std::string>		_return;

		typedef void(LocationConfig::*Setter)(const std::vector<std::string>&);
		std::map<std::string, Setter>	_directiveSetters;

		void	initDirectiveMap();

		void	setPath(const std::vector<std::string>& values);
		void	setMethods(const std::vector<std::string>& values);
		void	setReturn(const std::vector<std::string>& values);

	public:
		LocationConfig(std::string path);
		LocationConfig(const LocationConfig& other);
		~LocationConfig(void);

		void	parseLocation(const std::string key, const std::vector<std::string> values);

		const std::string& 					getPath(void) const;
		const std::set<std::string>&		getMethods(void) const;
		const std::pair<int, std::string>&	getReturn(void) const;

		bool				hasReturn(void) const;
		bool				isRedirectReturn(void) const;
		bool				isErrorReturn(void) const;
		int					getReturnCode(void) const;
		const std::string&	getReturnPath(void) const;
};

#endif