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

#include "parse/ParseCookie.hpp"
#include "utils/ParseUtils.hpp"

ParseCookie::ParseCookie() { }

ParseCookie::~ParseCookie() { }

// Valida nome do cookie
bool ParseCookie::validateName(const std::string& name) {
	// Valida tamanho: não pode estar vazio nem exceder MAX_COOKIE_NAME_SIZE
	if (name.empty() || name.size() > MAX_COOKIE_NAME_SIZE)
		return false;
	
	// Valida que contém apenas caracteres printáveis
	for (size_t i = 0; i < name.size(); ++i) {
		unsigned char c = static_cast<unsigned char>(name[i]);
		
		// Rejeita caracteres fora do intervalo
		if (c < 0x21 || c > 0x7E)
			return false;
		
		// Rejeita caracteres separadores: () <> @ , ; : \ " / [] ? = {}
		if (c == '(' || c == ')' || c == '<' || c == '>' || 
			c == '@' || c == ',' || c == ';' || c == ':' || 
			c == '\\' || c == '"' || c == '/' || c == '[' || 
			c == ']' || c == '?' || c == '=' || c == '{' || c == '}')
			return false;
	}
	return true;
}

// Valida valor do cookie
bool ParseCookie::validateValue(const std::string& value) {
	// Valida tamanho máximo
	if (value.size() > MAX_COOKIE_VALUE_SIZE)
		return false;
	
	//  Valida que contém apenas caracteres printáveis
	for (size_t i = 0; i < value.size(); ++i) {
		unsigned char c = static_cast<unsigned char>(value[i]);
		
		// Rejeita caracteres fora do intervalo
		if (c < 0x21 || c > 0x7E)
			return false;
		
		// Rejeita caracteres proibidos: " , ; "\\"
		if (c == '"' || c == ',' || c == ';' || c == '\\')
			return false;
	}
	return true;
}

// Valida tamanho total do header Cookie
bool ParseCookie::validateCookieSize(const std::string& cookie_header) {
	return cookie_header.size() <= MAX_COOKIE_SIZE;
}

// Parse do header Cookie no formato: "Cookie: name1=value1; name2=value2; name3=value3"
std::map<std::string, std::string> ParseCookie::parseCookie(const std::string& cookie_header) {
	std::map<std::string, std::string> cookies;
	
	// Valida tamanho total do Cookie
	if (!validateCookieSize(cookie_header))
		return cookies;
	
	std::istringstream stream(cookie_header);
	std::string pair;
	size_t cookie_count = 0;
	
	// Separa por ';' para obter cada par formato "key=value"
	while (std::getline(stream, pair, ';')) {
		// Limita quantidade máxima de cookies
		if (++cookie_count > MAX_COOKIES_COUNT)
			break;
		
		pair = ParseUtils::trim(pair);
		
		// Divide cada par em key/value ao encontrar '='
		size_t equal_pos = pair.find('=');
		if (equal_pos == std::string::npos)
			continue;
		
		std::string key = ParseUtils::trim(pair.substr(0, equal_pos));
		std::string value = ParseUtils::trim(pair.substr(equal_pos + 1));
		
		// Valida key e value individualmente
		if (!validateName(key))
			continue;
		if (!validateValue(value))
			continue;
		// Cookie válido, adiciona ao map
		cookies[key] = value;
	}
	return cookies;
}

bool ParseCookie::validateCookie(const std::string &cookie) {
	if (cookie.empty())
		return false;
	std::map<std::string, std::string> parsed = ParseCookie::parseCookie(cookie);
	return !parsed.empty();
}