/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseHttp.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cabo-ram <cabo-ram@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/31 14:44:10 by cabo-ram          #+#    #+#             */
/*   Updated: 2025/12/06 15:38:47 by cabo-ram         ###   ########.fr       */
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
#include <poll.h>
#include "../Response.hpp"
#include "../Request.hpp"
#include "ParseUri.hpp"
#include "ParseHttpValidator.hpp"
#include "ParseHttpReader.hpp"
#include "ParseUri.hpp"

// #define RECV_BUFFER_SIZE 4096

#define MAX_HEADER_SIZE 8192

class ParseHttp {
	private:
		RequestMethod _request_method;
		std::string _request_uri;
		std::string _request_path;
		std::string _request_path_info;
		std::string _query;
		std::string _http_version;
		std::string _host_header;
		std::string _user_agent_header;
		std::string _content_length;
		std::string _transfer_encoding;
		std::string _content_type;
		std::string _connection;
		std::string _accept;
		std::map<std::string, std::string> _cookies;
		std::string _request_body;
		std::map<std::string, std::string> _all_headers;

		bool parseRequestLine(const std::string &line,
			std::string &out_method,
			std::string &out_path,
			std::string &out_version);
		std::map<std::string,std::string> parseHeaders(const std::string &headers_block);

	public:
		ParseHttp();
		// ParseHttp(const ParseHttp &other);
		// ParseHttp &operator=(const ParseHttp &other);
		~ParseHttp();

		void toLowerStr(std::string &str);
		HttpStatus	initParse(std::string &request);
		Request		buildRequest() const;

		RequestMethod getRequestMethod() const;
		const std::string& getPath() const;
		const std::string& getHttpVersion() const;
		const std::string& getHost() const;
		const std::string& getUserAgent() const;
		const std::string& getBody() const;
		const std::string& getUri() const;
		const std::map<std::string, std::string>& getCookies() const;
};

RequestMethod stringToMethod(const std::string &method);
std::ostream& operator<<(std::ostream& os, RequestMethod method);

#endif