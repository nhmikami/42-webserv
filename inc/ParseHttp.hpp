/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseHttp.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cabo-ram <cabo-ram@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/31 14:44:10 by cabo-ram          #+#    #+#             */
/*   Updated: 2025/11/13 17:08:35 by cabo-ram         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSEHTTP_HPP
#define PARSEHTTP_HPP

#include <iostream>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <cctype>
#include <climits>
#include <sstream>
#include <map>
#include <vector>
#include <sys/socket.h>
#include "Response.hpp"
#include "Request.hpp"

class ParseHttp {
	private:
		RequestMethod request_method;
		std::string request_uri;
		std::string request_path;
		std::string query;
		std::string http_version;
		std::string host_header;
		std::string user_agent_header;
		std::string request_body;
		std::string content_length;
		std::string content_type;
		std::string transfer_encoding;
		std::string connection;
		std::string accept;
		std::map<std::string, std::string> all_headers;

		bool parse_request_line(const std::string &line,
			std::string &out_method,
			std::string &out_path,
			std::string &out_version);
		bool validate_uri(const std::string &uri, std::string &path, std::string &query);
		bool normalize_path(const std::string &raw_path, std::string &normalized_path);
		bool validate_host_header(const std::string &host);
		bool validate_content_length(const std::string &content_length_str, size_t &out_length);
		bool validate_transfer_encoding(const std::string &transfer_encoding);
		bool validate_content_type(const std::string &content_type);
		bool validate_connection(const std::string &connection);
		bool validate_quality_value(const std::string &s);
		double q_to_double(const std::string &s);
		bool validate_type_token(const std::string &t, bool is_type);
		bool check_params_q(const std::string &params_str, double &out_q, bool &has_q);
		bool validate_accept(const std::string &accept);
		HttpStatus validate_headers(const std::map<std::string, std::string> &headers);
		std::map<std::string,std::string> parse_headers(const std::string &headers_block);
		HttpStatus	read_body(int client_fd, size_t content_length);
		HttpStatus	read_chunked(int client_fd, std::string &buffer);
		bool read_until_crlf(int client_fd, std::string &buffer, std::string &out_line);
		bool hex_to_int(const std::string &hex_line, size_t &out_size);
	
	public:
		ParseHttp();
		ParseHttp(const ParseHttp &other);
		ParseHttp &operator=(const ParseHttp &other);
		~ParseHttp();

		HttpStatus	initParse(int client_fd, std::string &remaining_buffer, int max_header_size);
		Request		buildRequest() const;

		RequestMethod getRequestMethod() const;
		const std::string& getPath() const;
		const std::string& getHttpVersion() const;
		const std::string& getHost() const;
		const std::string& getUserAgent() const;
		const std::string& getBody() const;
		const std::string& getUri() const;
};

std::string trim(const std::string &s);
RequestMethod stringToMethod(const std::string &method);
std::ostream& operator<<(std::ostream& os, RequestMethod method);

#endif