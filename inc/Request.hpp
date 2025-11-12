/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cabo-ram <cabo-ram@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/06 15:02:39 by cabo-ram          #+#    #+#             */
/*   Updated: 2025/11/10 15:22:53 by cabo-ram         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <map>
#include <cstdlib>

enum RequestMethod {
	GET,
	POST,
	DELETE,
	UNKNOWN
};

class Request {
	private:
		RequestMethod method;
		std::string uri;
		std::string path;
		std::string query;
		std::string http_version;
		std::map<std::string, std::string> headers;
		std::string body;
		
	public:
		Request();
		Request(const Request &other);
		Request &operator=(const Request &other);
		~Request();
		void setMethod(RequestMethod m);
		void setUri(const std::string &u);
		void setPath(const std::string &p);
		void setQuery(const std::string &q);
		void setHttpVersion(const std::string &v);
		void setHeaders(const std::map<std::string, std::string> &h);
		void setBody(const std::string &b);
		void addHeader(const std::string &key, const std::string &value);

		RequestMethod getMethod() const;
		const std::string& getUri() const;
		const std::string& getPath() const;
		const std::string& getQuery() const;
		const std::string& getHttpVersion() const;
		const std::map<std::string, std::string>& getHeaders() const;
		const std::string& getBody() const;
		
		std::string getHeader(const std::string &key) const;
		bool hasHeader(const std::string &key) const;
		size_t getContentLength() const;
		bool isChunked() const;
		
		std::string getAcceptedContentTypes() const;
		std::string getConnection() const;
		std::string getUserAgent() const;
		std::string getHost() const;
		bool isKeepAlive() const;
		std::string getContentType() const;
		
		std::string getQueryParameter(const std::string &key) const;
		std::map<std::string, std::string> getQueryParameters() const;
		bool hasQueryParameter(const std::string &key) const;
		
		bool isValidForFileOperation() const;
		std::string getRequestTarget() const;
		bool requiresBody() const;
};

#endif