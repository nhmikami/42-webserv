#include <csignal>
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>

#include "Server.hpp"
#include "Logger.hpp"

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
        Server server;
        server.run();
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
