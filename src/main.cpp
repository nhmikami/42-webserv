#include "Server.hpp"

int main(int ac, char **av)
{
    std::string file;
    if (ac == 2)
        file = av[1];
    else
        file = "config/default.conf";

    try {
        // Ler arquivo de configuração
        Config config = ConfigParser::load("server.conf");
    
        // Criar o socket do servidor e configurar porta
        Server server(config);
    
        // Entrar no loop principal (event loop)
        server.run();
    } catch (std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}