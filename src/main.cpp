#include <csignal>
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>

#include "Server.hpp"
#include "Logger.hpp"
#include "ParseConfig.hpp"

void signalHandler(int signum) {
	(void)signum;
	throw std::runtime_error("Closing server!");
}

void printConfig(const std::vector<ServerConfig> &servers_config)
{
	for (size_t i = 0; i < servers_config.size(); i++)
	{
		std::cout << "========================================" << std::endl;
		std::cout << "Server " << i << ":" << std::endl;
		std::cout << "========================================" << std::endl;
		std::cout << "host: " << servers_config[i].getHost() << std::endl;
		std::cout << "port: " << servers_config[i].getPort() << std::endl;
		std::cout << "root: " << servers_config[i].getRoot() << std::endl;
		std::cout << "server name: " << servers_config[i].getServerName() << std::endl;
		std::cout << "autoindex: " << (servers_config[i].getAutoIndex() ? "on" : "off") << std::endl;
		std::cout << "client max body size: " << servers_config[i].getClientMaxBodySize() << std::endl;
		
		std::cout << "index files: ";
		const std::vector<std::string>& indexFiles = servers_config[i].getIndexFiles();
		for (size_t j = 0; j < indexFiles.size(); j++) {
			std::cout << indexFiles[j];
			if (j < indexFiles.size() - 1)
				std::cout << ", ";
		}
		std::cout << std::endl;

		std::cout << "error pages: ";
		const std::map<int, std::string>& errorPages = servers_config[i].getErrorPages();
		if (errorPages.empty()) {
			std::cout << "none";
		} else {
			for (std::map<int, std::string>::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it) {
				std::cout << it->first << "=" << it->second;
				std::map<int, std::string>::const_iterator next = it;
				if (++next != errorPages.end())
					std::cout << ", ";
			}
		}
		std::cout << std::endl;

		// Print locations
		std::map<std::string, LocationConfig> locations = servers_config[i].getLocations();
		std::cout << "\n  Locations (" << locations.size() << "):" << std::endl;
		for (std::map<std::string, LocationConfig>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
			std::cout << "  ----------------------------------------" << std::endl;
			std::cout << "  Location " << it->first << ":" << std::endl;
			std::cout << "  path: " << it->second.getPath() << std::endl;
			std::cout << "  root: " << it->second.getRoot() << std::endl;
			std::cout << "  autoindex: " << (it->second.getAutoIndex() ? "on" : "off") << std::endl;
			std::cout << "  client max body size: " << it->second.getClientMaxBodySize() << std::endl;
			
			std::cout << "  methods: ";
			const std::set<std::string>& methods = it->second.getMethods();

			for (std::set<std::string>::const_iterator method_it = methods.begin(); method_it != methods.end(); ++method_it) {
				std::cout << *method_it;
				std::set<std::string>::const_iterator next = method_it;
				if (++next != methods.end())
					std::cout << ", ";
			}
			std::cout << std::endl;

			std::cout << "  index files: ";
			const std::vector<std::string>& locIndexFiles = it->second.getIndexFiles();
			for (size_t k = 0; k < locIndexFiles.size(); k++) {
				std::cout << locIndexFiles[k];
				if (k < locIndexFiles.size() - 1)
					std::cout << ", ";
			}
			std::cout << std::endl;

			std::cout << "  error pages: ";
			const std::map<int, std::string>& locErrorPages = it->second.getErrorPages();
			if (locErrorPages.empty()) {
				std::cout << "none";
			} else {
				for (std::map<int, std::string>::const_iterator error_it = locErrorPages.begin(); error_it != locErrorPages.end(); ++error_it) {
					std::cout << error_it->first << "=" << error_it->second;
					std::map<int, std::string>::const_iterator next = error_it;
					if (++next != locErrorPages.end())
						std::cout << ", ";
				}
			}
			std::cout << std::endl;

			std::cout << "  cgi: ";
			const std::map<std::string, std::string>& cgiMap = it->second.getCgi();
			if (cgiMap.empty()) {
				std::cout << "none";
			} else {
				for (std::map<std::string, std::string>::const_iterator cgi_it = cgiMap.begin(); cgi_it != cgiMap.end(); ++cgi_it) {
					std::cout << cgi_it->first << " -> " << cgi_it->second;
					std::map<std::string, std::string>::const_iterator next = cgi_it;
					if (++next != cgiMap.end())
						std::cout << ", ";
				}
			}
			std::cout << std::endl;
		}

		std::cout << std::endl;
	}
}

int main(int ac, char **av) {
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);

	try {
		if (ac != 2) {
			throw std::invalid_argument("Usage: ./webserv <config_file>");
		}

		ParseConfig parser(av[1]);

		std::vector<ServerConfig> configs = parser.parse();

		printConfig(configs);
	   
		Server server(configs);
		server.run();
	}
	catch (const std::runtime_error& e) {
		Logger::log(Logger::INFO, e.what());
	}
	catch (const std::exception& e) {
		Logger::log(Logger::ERROR, e.what());
		return 1;
	}

	return 0;
}
