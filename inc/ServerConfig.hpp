#ifndef SERVER_CONFIG_HPP
#define SERVER_CONFIG_HPP

#include "Request.hpp"
#include "Response.hpp"
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <map>
#include <vector>

class ServerConfig {
	private:
		bool		_autoindex;
		std::string	_root;
		std::map<std::string, std::string> _index_list;

	public:
		ServerConfig(void) : _autoindex(true) {}
		~ServerConfig(void) {}

		bool getAutoindex() const {
			return _autoindex;
		}

		std::string getRoot() const {
			return _root;
		}

		// void run() {
		// 	createListeningSocket(config.port);

		// 	while (true) {
		// 		// Esperar eventos de leitura/escrita (poll)
		// 		poll_fds = buildPollSet(socket_fd, clients);
		// 		poll(poll_fds);

		// 		// Se chegou nova conexão → aceitar
		// 		if (pollEventOn(socket_fd))
		// 			acceptNewClient();

		// 		// Se um cliente enviou dados → ler requisição
		// 		for (client in clients) {
		// 			if (pollEventOn(client.fd)) {
		// 				std::string raw_request = client.read();
		// 				if (!raw_request.empty())
		// 					handleRequest(client, raw_request);
		// 			}
		// 		}
		// 	}
		// }

		void handleRequest() {
			// parse request

			// get response

			// send response to client
		}
};

#endif