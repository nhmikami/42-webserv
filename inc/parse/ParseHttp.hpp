#ifndef PARSEHTTP_HPP
# define PARSEHTTP_HPP

# include <iostream>
# include <cstdlib>
# include <cerrno>
# include <cstring>
# include <cctype>
# include <climits>
# include <sstream>
# include <map>
# include <vector>

# include "http/Response.hpp"
# include "http/Request.hpp"
# include "config/BaseConfig.hpp"
# include "ParseUri.hpp"
# include "ParseHttpValidator.hpp"
# include "ParseHttpReader.hpp"
# include "parse/ParseCookie.hpp"
# include "utils/ParseUtils.hpp"

# define MAX_HEADER_SIZE 8192

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
		size_t _max_body_size;

		bool parseRequestLine(const std::string &line,
			std::string &out_method,
			std::string &out_path,
			std::string &out_version);
		std::map<std::string,std::string> parseHeaders(const std::string &headers_block);

	public:
		ParseHttp();
		ParseHttp(const ParseHttp &other);
		ParseHttp &operator=(const ParseHttp &other);
		~ParseHttp();

		void toLowerStr(std::string &str);
		void setMaxBodySize(size_t max_body_size);
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