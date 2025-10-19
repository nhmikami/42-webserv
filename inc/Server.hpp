#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include "Config.hpp"
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>

class Server {
	private:
	int socket_fd;
	std::vector<Client> clients;
	Config config;

	public:
		Server(Config const &conf) : config(conf) {}

		void run() {
			createListeningSocket(config.port);

			while (true) {
				// Esperar eventos de leitura/escrita (poll)
				poll_fds = buildPollSet(socket_fd, clients);
				poll(poll_fds);

				// Se chegou nova conexão → aceitar
				if (pollEventOn(socket_fd))
					acceptNewClient();

				// Se um cliente enviou dados → ler requisição
				for (client in clients) {
					if (pollEventOn(client.fd)) {
						std::string raw_request = client.read();
						if (!raw_request.empty())
							handleRequest(client, raw_request);
					}
				}
			}
		}

		void handleRequest(Client &client, std::string const &raw_request) {
			// Interpretar requisição HTTP
			HTTPRequest req = HTTPParser::parse(raw_request);

			// Gerar resposta
			HTTPResponse res = Router::handle(req, config);

			// Enviar resposta ao cliente
			client.send(res.toString());
		}
};

#endif