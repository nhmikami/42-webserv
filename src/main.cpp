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

int main(int ac, char **av) {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    Logger logger;
    (void)av;

    try {
        if (ac != 2) {
            throw std::invalid_argument("Usage: ./webserv <config_file>");
        }

        ParseConfig parser(av[1]);

        std::vector<ServerConfig> servers_config = parser.parse();
        for (size_t i = 0; i < servers_config.size(); i++)
        {
            std::cout << "Server " << i << ":" << std::endl;
            std::cout << "host: " << servers_config[i].getHost() << std::endl;
            std::cout << "port: " << servers_config[i].getPort() << std::endl;
            std::cout << "root: " << servers_config[i].getRoot() << std::endl;
            std::cout << "server name: " << servers_config[i].getServerName() << std::endl;
            std::cout << "index files: " << servers_config[i].getIndexFiles() << std::endl;
            std::cout << "error pages: " << servers_config[i].getErrorPages() << std::endl;
            std::cout << "client max body size: " << servers_config[i].getClientaMaxBodySize() << std::endl;
            std::cout << std::endl;
        }
        // Server server;
        // server.run();
    }
    catch (const std::runtime_error& e) {
        logger.log(Logger::INFO, e.what());
    }
    catch (const std::exception& e) {
        logger.log(Logger::ERROR, e.what());
        return 1;
    }

    return 0;
}
