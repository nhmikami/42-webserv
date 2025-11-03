#include "Server.hpp"
#include <iostream>
#include <string>

int main(int ac, char **av)
{
    std::string file = (ac > 1) ? av[1] : "/config/default.conf";

    try {
        Server server(file);

        //server.run();
    } catch (std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}