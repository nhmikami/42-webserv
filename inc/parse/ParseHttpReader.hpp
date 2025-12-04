/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseHttpReader.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/24 17:30:00 by marvin            #+#    #+#             */
/*   Updated: 2025/11/24 17:30:00 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSEHTTPREADER_HPP
#define PARSEHTTPREADER_HPP

#include <string>
#include <sys/socket.h>
#include <poll.h>
#include <cerrno>
#include <cstdlib>
#include "Response.hpp"

class ParseHttpReader {
	private:
		ParseHttpReader(void);
		~ParseHttpReader(void);

	public:
		// static bool readUntilCrlf(int client_fd, std::string &buffer, std::string &out_line);
		// static bool hexToInt(const std::string &hex_line, size_t &out_size);
		// static HttpStatus readBody(size_t content_length, std::string &request_body);
		// static HttpStatus readChunked(std::string &buffer, std::string &request_body);
};

#endif
