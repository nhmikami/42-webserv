#include "Server.hpp"
#include <iostream>
#include <string>

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <csignal>


void signalHandler(int signum) {
    (void)signum;
    throw std::runtime_error("Closing server!");
}

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    try {
        Server server;
        server.run();
    }
    catch (const std::runtime_error& e) {
        std::cout << "\n[SERVER] " << e.what() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
