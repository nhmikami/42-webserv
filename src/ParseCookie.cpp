/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseCookie.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 15:16:03 by marvin            #+#    #+#             */
/*   Updated: 2025/11/25 15:16:03 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/ParseCookie.hpp"
#include "utils/ParseUtils.hpp"

ParseCookie::ParseCookie() { }

ParseCookie::~ParseCookie() { }

// Valida nome do cookie
bool ParseCookie::validate_name(const std::string& name) {
	if (name.empty() || name.size() > MAX_COOKIE_NAME_SIZE)
		return false;
	for (size_t i = 0; i < name.size(); ++i) {
		unsigned char c = static_cast<unsigned char>(name[i]);
		if (c < 0x21 || c > 0x7E)
			return false;
		if (c == '(' || c == ')' || c == '<' || c == '>' || 
			c == '@' || c == ',' || c == ';' || c == ':' || 
			c == '\\' || c == '"' || c == '/' || c == '[' || 
			c == ']' || c == '?' || c == '=' || c == '{' || c == '}')
			return false;
	}
	return true;
}

// Valida valor do cookie
bool ParseCookie::validate_value(const std::string& value) {
	if (value.size() > MAX_COOKIE_VALUE_SIZE)
		return false;
	for (size_t i = 0; i < value.size(); ++i) {
		unsigned char c = static_cast<unsigned char>(value[i]);
		if (c < 0x21 || c > 0x7E)
			return false;
		if (c == '"' || c == ',' || c == ';' || c == '\\')
			return false;
	}
	return true;
}

// Valida tamanho total do header Cookie
bool ParseCookie::validate_cookie_size(const std::string& cookie_header) {
	return cookie_header.size() <= MAX_COOKIE_SIZE;
}

// Parse do header Cookie no formato: "Cookie: name1=value1; name2=value2; name3=value3"
std::map<std::string, std::string> ParseCookie::parse_cookie(const std::string& cookie_header) {
	std::map<std::string, std::string> cookies;
	
	if (!validate_cookie_size(cookie_header))
		return cookies;
	std::istringstream stream(cookie_header);
	std::string pair;
	size_t cookie_count = 0;
	
	while (std::getline(stream, pair, ';')) {
		if (++cookie_count > MAX_COOKIES_COUNT)
			break;
		pair = ParseUtils::trim(pair);
		size_t equal_pos = pair.find('=');
		if (equal_pos == std::string::npos)
			continue;
		std::string key = ParseUtils::trim(pair.substr(0, equal_pos));
		std::string value = ParseUtils::trim(pair.substr(equal_pos + 1));
		if (!validate_name(key))
			continue;
		if (!validate_value(value))
			continue;
		cookies[key] = value;
	}
	return cookies;
}
