#include "Server.hpp"
#include <fstream>

Server::Server(void) {};

Server::Server(const Server &other){};

Server::Server(std::string file_path): _file_path(file_path) {};

Server::~Server(void){};

Server &Server::operator=(const Server &other){};