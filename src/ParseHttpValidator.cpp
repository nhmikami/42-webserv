/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseHttpValidator.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/24 17:30:00 by marvin            #+#    #+#             */
/*   Updated: 2025/11/24 17:30:00 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ParseHttpValidator.hpp"
#include "../inc/ParseUtils.hpp"

ParseHttpValidator::ParseHttpValidator(void) {}

ParseHttpValidator::~ParseHttpValidator(void) {}

// valida o header host
bool ParseHttpValidator::validate_host_header(const std::string &host) {
	if (host.empty())
		return false;
	
	if (host.size() > 255)
		return false;
	
	size_t colon_pos = host.find(':');
	std::string hostname;
	std::string port;
	
	if (colon_pos != std::string::npos) {
		hostname = host.substr(0, colon_pos);
		port = host.substr(colon_pos + 1);
		
		if (port.empty())
			return false;
		for (size_t i = 0; i < port.size(); ++i) {
			if (!std::isdigit(static_cast<unsigned char>(port[i])))
				return false;
		}
		int port_num = std::atoi(port.c_str());
		if (port_num < 1 || port_num > 65535)
			return false;
	}
	else
		hostname = host;
	
	if (hostname.empty())
		return false;
	
	for (size_t i = 0; i < hostname.size(); ++i) {
		unsigned char c = static_cast<unsigned char>(hostname[i]);
		if (!std::isalnum(c) && c != '-' && c != '.')
			return false;
	}
	
	return true;
}

// valida o content_length e se é numérico
bool ParseHttpValidator::validate_content_length(const std::string &content_length_str, size_t &out_length) {
	if (content_length_str.empty())
		return false;
	
	for (size_t i = 0; i < content_length_str.size(); ++i) {
		if (!std::isdigit(static_cast<unsigned char>(content_length_str[i])))
			return false;
	}
	
	char *end_ptr = NULL;
	errno = 0;
	unsigned long value = std::strtoul(content_length_str.c_str(), &end_ptr, 10);
	
	if (errno == ERANGE)
		return false;
	
	if (end_ptr != content_length_str.c_str() + content_length_str.size())
		return false;
	
	out_length = static_cast<size_t>(value);
	return true;
}

// normaliza o transfer encoding
bool ParseHttpValidator::validate_transfer_encoding(const std::string &transfer_encoding) {
	if (transfer_encoding.empty())
		return false;

	std::string transf_encod_trim = ParseUtils::trim(transfer_encoding);
	
	if (transf_encod_trim == "chunked" || transf_encod_trim == "identity")
		return true;
	
	return false;
}

bool ParseHttpValidator::validate_content_type(const std::string &content_type) {
	if (content_type.empty())
		return false;
	
	size_t slash_pos = content_type.find('/');
	if (slash_pos == std::string::npos || slash_pos == 0)
		return false;
	
	size_t semicolon_pos = content_type.find(';');
	std::string mime_type;
	
	if (semicolon_pos != std::string::npos)
		mime_type = content_type.substr(0, semicolon_pos);
	else
		mime_type = content_type;
	
	mime_type = ParseUtils::trim(mime_type);
	
	slash_pos = mime_type.find('/');
	if (slash_pos == std::string::npos || slash_pos == 0 || slash_pos == mime_type.size() - 1)
		return false;
	
	for (size_t i = 0; i < mime_type.size(); ++i) {
		unsigned char c = static_cast<unsigned char>(mime_type[i]);
		if (!std::isalnum(c) && c != '/' && c != '-' && c != '+' && c != '.')
			return false;
	}
	
	return true;
}

bool ParseHttpValidator::validate_quality_value(const std::string &s) {
	if (s.empty())
		return false;
	for (size_t i = 0; i < s.size(); ++i) {
		if (std::isspace(static_cast<unsigned char>(s[i])))
			return false;
	}

	if (s[0] == '1') {
		if (s.size() == 1)
			return true;
		if (s[1] != '.')
			return false;
		size_t zeros = s.size() - 2;
		if (zeros == 0 || zeros > 3)
			return false;
		for (size_t i = 2; i < s.size(); ++i) {
			if (s[i] != '0')
				return false;
		}
		return true;
	}
	
	if (s[0] == '0') {
		if (s.size() == 1)
			return true;
		if (s[1] != '.')
			return false;
		size_t digits = s.size() - 2;
		if (digits < 1 || digits > 3)
			return false;
		for (size_t i = 2; i < s.size(); ++i) {
			if (!std::isdigit(static_cast<unsigned char>(s[i])))
				return false;
		}
		return true;
	}
	return false;
}

double ParseHttpValidator::q_to_double(const std::string &s) {
	return std::strtod(s.c_str(), NULL);
}

bool ParseHttpValidator::validate_type_token(const std::string &t, bool is_type) {
	(void)is_type;
	if (t.empty())
		return false;
	for (size_t i = 0; i < t.size(); ++i) {
		unsigned char c = static_cast<unsigned char>(t[i]);
		if (c == '*') {
			if (t.size() != 1)
				return false;
			return true;
		}
		if (!std::isalnum(c) && c != '-' && c != '+' && c != '.')
			return false;
	}
	return true;
}

bool ParseHttpValidator::check_params_q(const std::string &params_str, double &out_q, bool &has_q) {
	has_q = false;
	out_q = 1.0;
	if (params_str.empty())
		return true;

	size_t pos = 0;
	while (pos < params_str.size()) {
		size_t next = params_str.find(';', pos);
		std::string param;
		if (next == std::string::npos)
			param = params_str.substr(pos);
		else
			param = params_str.substr(pos, next - pos);

		param = ParseUtils::trim(param);
		if (param.empty())
			return false;

		size_t eq = param.find('=');
		if (eq == std::string::npos)
			return false;

		std::string name = param.substr(0, eq);
		std::string value = param.substr(eq + 1);
		name = ParseUtils::trim(name);
		value = ParseUtils::trim(value);

		for (size_t i = 0; i < name.size(); ++i)
			name[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(name[i])));

		if (name == "q") {
			if (has_q)
				return false;
			if (!validate_quality_value(value))
				return false;
			out_q = q_to_double(value);
			has_q = true;
		}
		if (next == std::string::npos)
			break;
		pos = next + 1;
	}
	return true;
}

bool ParseHttpValidator::validate_accept(const std::string &accept) {
	if (accept.empty())
		return false;
	
	std::string accept_trimmed = ParseUtils::trim(accept);
	if (accept_trimmed.empty())
		return false;
	
	if (accept_trimmed == "*/*")
		return true;
	
	size_t start = 0;
	while (start < accept_trimmed.size()) {
		size_t comma_pos = accept_trimmed.find(',', start);
		std::string media_type;
		
		if (comma_pos == std::string::npos)
			media_type = accept_trimmed.substr(start);
		else
			media_type = accept_trimmed.substr(start, comma_pos - start);
		
		media_type = ParseUtils::trim(media_type);
		
		std::string media_and_params = media_type;
		std::string params_str;
		size_t semicolon_pos = media_and_params.find(';');
		if (semicolon_pos != std::string::npos) {
			params_str = media_and_params.substr(semicolon_pos + 1);
			media_and_params = media_and_params.substr(0, semicolon_pos);
		}
		media_and_params = ParseUtils::trim(media_and_params);
		params_str = ParseUtils::trim(params_str);

		if (media_and_params.empty())
			return false;
		
		size_t slash_pos = media_and_params.find('/');

		if (slash_pos == std::string::npos || slash_pos == 0)
			return false;
		
		std::string type = media_and_params.substr(0, slash_pos);
		std::string subtype = media_and_params.substr(slash_pos + 1);
		if (subtype.empty())
			return false;
		if (type == "*" && subtype != "*")
			return false;
		if (!validate_type_token(type, true))
			return false;
		if (!validate_type_token(subtype, false))
			return false;

		double q_value = 1.0;
		bool has_q = false;
		if (!check_params_q(params_str, q_value, has_q))
			return false;
		
		if (comma_pos == std::string::npos)
			break;
		start = comma_pos + 1;
	}
	
	return true;
}

bool ParseHttpValidator::validate_connection(const std::string &connection) {
	if (connection.empty())
		return false;
	
	std::string connec_trim = ParseUtils::trim(connection);
	
	if (connec_trim == "keep-alive" || connec_trim == "close")
		return true;
	
	return false;
}

// Valida regras HTTP mínimas e coerência entre headers antes de aceitar/processar o corpo ou rotear a requisição
HttpStatus ParseHttpValidator::validate_headers(const std::map<std::string, std::string> &headers) {
	std::map<std::string, std::string>::const_iterator host_it = headers.find("host");
	if (host_it == headers.end())
		return BAD_REQUEST;
	
	if (!validate_host_header(host_it->second))
		return BAD_REQUEST;
	
	std::map<std::string, std::string>::const_iterator content_length_it = headers.find("content-length");
	std::map<std::string, std::string>::const_iterator transfer_encoding_it = headers.find("transfer-encoding");
	bool has_content_length = (content_length_it != headers.end());
	bool has_transfer_encoding = (transfer_encoding_it != headers.end());
	
	if (has_content_length && has_transfer_encoding)
		return BAD_REQUEST;
	
	if (has_content_length) {
		size_t content_length;
		if (!validate_content_length(content_length_it->second, content_length))
			return BAD_REQUEST;
	}
	
	if (has_transfer_encoding) {
		if (!validate_transfer_encoding(transfer_encoding_it->second))
			return BAD_REQUEST;
	}
	
	std::map<std::string, std::string>::const_iterator content_type_it = headers.find("content-type");
	if (content_type_it != headers.end()) {
		if (!validate_content_type(content_type_it->second))
			return BAD_REQUEST;
	}
	
	std::map<std::string, std::string>::const_iterator connection_it = headers.find("connection");
	if (connection_it != headers.end()) {
		if (!validate_connection(connection_it->second))
			return BAD_REQUEST;
	}
	
	std::map<std::string, std::string>::const_iterator accept_it = headers.find("accept");
	if (accept_it != headers.end())
		if (!validate_accept(accept_it->second))
			return BAD_REQUEST;
	
	return OK;
}
