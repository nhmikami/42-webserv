#include "Server.hpp"
#include <iostream>
#include <string>

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>

int main() {
    const char* HOST = "127.0.0.1";
    const int PORT = 8080;

    int server_fd, client_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket failed");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(HOST);
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen failed");
        close(server_fd);
        return 1;
    }

    std::cout << "[SERVER] Listening on " << HOST << ":" << PORT << std::endl;

    while (true) {
        client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (client_fd < 0) {
            perror("accept failed");
            continue;
        }

        std::cout << "[SERVER] Client connected!" << std::endl;

        char buffer[4096] = {0};
        int bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes > 0) {
            std::cout << "[SERVER] Received:\n" << buffer << std::endl;
        }

        // Aqui eu envio os bytes para o parsing e recebo uma resposta a qual devolverei para o cliente
        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 13\r\n"
            "\r\n"
            "Hello, World!";

        send(client_fd, response.c_str(), response.size(), 0);
        close(client_fd);
        std::cout << "[SERVER] Connection closed.\n";
    }

    close(server_fd);
    return 0;
}


// int main(int ac, char **av)
// {
//     std::string file = (ac > 1) ? av[1] : "/config/default.conf";

//     try {
//         Server server(file);

//         //server.run();
//     } catch (std::exception &e) {
//         std::cerr << "Error: " << e.what() << std::endl;
//         return 1;
//     }

//     return 0;
// }