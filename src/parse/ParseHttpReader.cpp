/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseHttpReader.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/24 17:30:00 by marvin            #+#    #+#             */
/*   Updated: 2025/11/24 17:30:00 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../inc/ParseHttpReader.hpp"
#include "../../inc/utils/ParseUtils.hpp"
#include "../../inc/ParseHttp.hpp"

ParseHttpReader::ParseHttpReader(void) {}

ParseHttpReader::~ParseHttpReader(void) {}

// // lê os dados do socket até encontrar "\r\n"
// bool ParseHttpReader::readUntilCrlf(int client_fd, std::string &buffer, std::string &out_line) {
// 	static const size_t MAX_LINE = 16 * 1024;
// 	static const int TIMEOUT_MS = 5000;
	
// 	while (true) {
// 		size_t crlf_pos = buffer.find("\r\n");
// 		if (crlf_pos != std::string::npos) {
// 			out_line = buffer.substr(0, crlf_pos);
// 			buffer.erase(0, crlf_pos + 2);
// 			return true;
// 		}
		
// 		if (buffer.size() > MAX_LINE)
// 			return false;
		
// 		// struct pollfd pfd = {client_fd, POLLIN, 0};

// 		// int poll_result = poll(&pfd, 1, TIMEOUT_MS);
		
// 		// if (poll_result == 0)
// 		// 	return false;
// 		// if (poll_result < 0) {
// 		// 	if (errno == EINTR)
// 		// 		continue;
// 		// 	return false;
// 		// }
// 		// if (poll_result > 0) {
// 		// 	for (int i = 0; i < 2; ++i) {
// 		// 		if (pfd.revents & POLLERR || POLLHUP ||POLLNVAL)
// 		// 			return false;
// 		// 	}
// 		// 	if (!(pfd.revents & POLLIN))
// 		// 		continue;
// 		// }
		
// 		char temp[RECV_BUFFER_SIZE];
// 		ssize_t bytes_received = recv(client_fd, temp, sizeof(temp), 0);
		
// 		if (bytes_received <= 0)
// 			return false;
		
// 		// Verifica tamanho antes de adicionar
// 		if (buffer.size() + static_cast<size_t>(bytes_received) > MAX_LINE)
// 			return false;
// 		buffer.append(temp, static_cast<size_t>(bytes_received));
// 	}
// }

// // converte hexadecimal para inteiro (tamanho do chunk)
// bool ParseHttpReader::hexToInt(const std::string &hex_line, size_t &out_size) {
// 	std::string hex_str = hex_line;
	
// 	size_t semicolon_pos = hex_str.find(";");
// 	if (semicolon_pos != std::string::npos)
// 		hex_str = hex_str.substr(0, semicolon_pos);
	
// 	hex_str = ParseUtils::trim(hex_str);
// 	if (hex_str.empty())
// 		return false;
	
// 	char *end_ptr = NULL;
// 	errno = 0;
// 	unsigned long hex_value = strtoul(hex_str.c_str(), &end_ptr, 16);
	
// 	if (end_ptr == hex_str.c_str())
// 		return false;
	
// 	if (errno == ERANGE)
// 		return false;
	
// 	out_size = static_cast<size_t>(hex_value);
// 	return true;
// }

// HttpStatus ParseHttpReader::readBody(size_t content_length, std::string &request_body) {
// 	static const size_t MAX_TOTAL = 10 * 1024 * 1024;
	
// 	if (content_length == 0)
// 		return OK;
	
// 	if (request_body.size() + content_length > MAX_TOTAL)
// 		return PAYLOAD_TOO_LARGE;
	
// 	char buffer[RECV_BUFFER_SIZE];
// 	ssize_t	bytes_read;
// 	size_t	total_read = 0;
// 	request_body.reserve(request_body.size() + content_length);

// 	while (total_read < content_length) {
// 		size_t bytes_to_read = content_length - total_read;
// 		if (bytes_to_read > RECV_BUFFER_SIZE)
// 			bytes_to_read = RECV_BUFFER_SIZE;
		
// 		bytes_read = recv(client_fd, buffer, bytes_to_read, 0);
		
// 		if (bytes_read <= 0)
// 			return BAD_REQUEST;
		
// 		if (static_cast<size_t>(bytes_read) > MAX_TOTAL - total_read)
// 			return PAYLOAD_TOO_LARGE;
		
// 		request_body.append(buffer, static_cast<size_t>(bytes_read));
// 		total_read += static_cast<size_t>(bytes_read);
// 	}
// 	return OK;
// }

// HttpStatus ParseHttpReader::readChunked(std::string &buffer, std::string &request_body) {
// 	request_body.clear();
// 	static const size_t MAX_TOTAL = 10 * 1024 * 1024;
// 	size_t total_bytes_read = 0;
	
// 	while (true) {
// 		std::string chunk_size_line;
// 		if (!readUntilCrlf(client_fd, buffer, chunk_size_line))
// 			return BAD_REQUEST;
		
// 		size_t chunk_size;
// 		if (!hexToInt(chunk_size_line, chunk_size))
// 			return BAD_REQUEST;
		
// 		if (chunk_size == 0) {
// 			while (true) {
// 				std::string trailer_line;
// 				if (!readUntilCrlf(client_fd, buffer, trailer_line))
// 					return BAD_REQUEST;
// 				if (trailer_line.empty())
// 					return OK;
// 			}
// 		}
		
// 		if (chunk_size > MAX_TOTAL - total_bytes_read)
// 			return PAYLOAD_TOO_LARGE;
		
// 		while (buffer.size() < chunk_size) {
// 			char temp[RECV_BUFFER_SIZE];
// 			ssize_t bytes_received = recv(client_fd, temp, sizeof(temp), 0);
			
// 			if (bytes_received <= 0)
// 				return BAD_REQUEST;
			
// 			buffer.append(temp, static_cast<size_t>(bytes_received));
// 		}
		
// 		request_body.append(buffer.substr(0, chunk_size));
// 		buffer.erase(0, chunk_size);
// 		total_bytes_read += chunk_size;
		
// 		while (buffer.size() < 2) {
// 			char temp[RECV_BUFFER_SIZE];
// 			ssize_t bytes_received = recv(client_fd, temp, sizeof(temp), 0);
			
// 			if (bytes_received <= 0)
// 				return BAD_REQUEST;
			
// 			buffer.append(temp, static_cast<size_t>(bytes_received));
// 		}
		
// 		if (buffer.size() < 2 || buffer[0] != '\r' || buffer[1] != '\n')
// 			return BAD_REQUEST;
// 		buffer.erase(0, 2);
// 	}
// }
