#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <map>
#include <utility>

#include "BaseConfig.hpp"
#include "config/LocationConfig.hpp"
#include "utils/ParseUtils.hpp"

class ServerConfig : public BaseConfig {
	private:
		std::string								_host;
		int										_port;
		std::string								_server_name;
		std::map<std::string, LocationConfig>	_locations;

		typedef void(ServerConfig::*Setter)(const std::vector<std::string>&);
		std::map<std::string, Setter>			_directiveSetters;

		void	initDirectiveMap();

		void	setListen(const std::vector<std::string>& values);
		void	setHost(const std::vector<std::string>& values);
		void	setServerName(const std::vector<std::string>& values);


	public:
		ServerConfig(void);
		~ServerConfig(void);

		void	parseServer(const std::string key, const std::vector<std::string> values);
		void	addLocation(const std::vector<std::string>&values, std::string *location_path);

		const std::string							getHost(void) const;
		int											getPort(void) const;
		const std::string 							getServerName(void) const;
		const std::map<std::string, LocationConfig>	getLocations(void) const;
		LocationConfig*								getLocation(const std::string path);
};

const LocationConfig* ServerConfig::_findLocation(const std::string& path) {
	const std::map<std::string, LocationConfig> &locations = _config.getLocations();
	const LocationConfig* bestMatch = NULL;
	std::string bestKey = "";

	for (std::map<std::string, LocationConfig>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
		const std::string &key = it->first;

		if (key.size() > path.size())
			continue;

		if (path.compare(0, key.size(), key) == 0) {
			if (key.size() == path.size() || path[key.size()] == '/' || key == "/") {
				if (key.size() > bestKey.size()) {
					bestMatch = &(it->second);
					bestKey = key;
				}
			}
		}
	}
	return bestMatch;
}

#endif