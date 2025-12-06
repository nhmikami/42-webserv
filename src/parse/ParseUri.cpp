/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseUri.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/24 17:30:00 by marvin            #+#    #+#             */
/*   Updated: 2025/11/24 17:30:00 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parse/ParseUri.hpp"

ParseUri::ParseUri(void) {}

ParseUri::~ParseUri(void) {}

int ParseUri::hexDigit(char c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	return -1;
}

int ParseUri::hexValue(char hi, char lo) {
	int	high = hexDigit(hi);
	int	low  = hexDigit(lo);

	if (high < 0 || low < 0)
		return -1;
	return (high * 16) + low;
}

bool ParseUri::isValidUTF8(const std::string &s) {
	const unsigned char* bytes = reinterpret_cast<const unsigned char*>(s.c_str());
	const size_t n = s.size();
	size_t i = 0;

	while (i < n) {
		unsigned char c = bytes[i];
		// ASCII de 1 byte (0x00-0x7F)
		if (c <= 0x7F) {
			++i;
			continue;
		}
		// 2 bytes (0xC2-0xDF)
		if (c >= 0xC2 && c <= 0xDF) {
			if (i + 1 >= n || (bytes[i + 1] & 0xC0) != 0x80)
				return false;
			i += 2;
			continue;
		}
		// 3 bytes (0xE0-0xEF)
		if (c >= 0xE0 && c <= 0xEF) {
			if (i + 2 >= n)
				return false;
			unsigned char c1 = bytes[i + 1];
			unsigned char c2 = bytes[i + 2];
			
			if ((c2 & 0xC0) != 0x80)
				return false;
			if (c == 0xE0) {
				if (c1 < 0xA0 || c1 > 0xBF)
					return false;
			}
			else if (c == 0xED) {
				if (c1 < 0x80 || c1 > 0x9F)
					return false;
			}
			else {
				if ((c1 & 0xC0) != 0x80)
					return false;
			}
			i += 3;
			continue;
		}
		// 4 bytes (0xF0-0xF4)
		if (c >= 0xF0 && c <= 0xF4) {
			if (i + 3 >= n)
				return false;
			unsigned char c1 = bytes[i + 1];
			unsigned char c2 = bytes[i + 2];
			unsigned char c3 = bytes[i + 3];
			// checking bits format
			if (((c2 | c3) & 0xC0) != 0x80)
				return false;
			if (c == 0xF0) {
				if (c1 < 0x90 || c1 > 0xBF)
					return false;
			}
			else if (c == 0xF4) {
				if (c1 < 0x80 || c1 > 0x8F)
					return false;
			}
			else {
				if ((c1 & 0xC0) != 0x80)
					return false;
			}
			i += 4;
			continue;
		}
		return false;
	}
	return true;
}

// Decodifica URL encoded PATH (percent-encoding)
bool ParseUri::urlDecodePath(const std::string &str, std::string &result) {
	result.clear();
	for (size_t i = 0; i < str.length(); ++i) {
		if (str[i] == '%') {
			// Valida formato %XX
			if (i + 2 >= str.length())
				return false;
			
			int value = hexValue(str[i + 1], str[i + 2]);
			if (value < 0)
				return false;
			
			// Rejeita %00 (null byte)
			if (value == 0)
				return false;
			
			// Rejeita caracteres não printáveis
			if (value < 0x20)
				return false;
			
			// Não decodifica %2F (/) no path
			if (value == 0x2F) {
				result.push_back('%');
				result.push_back(str[i + 1]);
				result.push_back(str[i + 2]);
				i += 2;
				continue;
			}
			result.push_back(static_cast<char>(value));
			i += 2;
		}
		else {
			unsigned char uc = static_cast<unsigned char>(str[i]);
			// Rejeita caracteres de controle não codificados
			if (uc < ' ')
				return false;
			result.push_back(str[i]);
		}
	}
	if (!isValidUTF8(result))
		return false;
	return true;
}

// Decodifica URL encoded QUERY (percent-encoding)
bool ParseUri::urlDecodeQuery(const std::string &str, std::string &result) {
	result.clear();
	for (size_t i = 0; i < str.length(); ++i) {
		if (str[i] == '%') {
			// Valida formato %XX
			if (i + 2 >= str.length())
				return false;
			
			int value = hexValue(str[i + 1], str[i + 2]);
			if (value < 0) 
				return false;
			
			// Rejeita %00 (null byte)
			if (value == 0)
				return false;
			
			// Rejeita caracteres não printáveis
			if (value < 0x20)
				return false;
			
			// Decodifica tudo na query (incluindo %2F)
			result.push_back(static_cast<char>(value));
			i += 2;
		}
		else {
			unsigned char uc = static_cast<unsigned char>(str[i]);
			// Rejeita caracteres de controle não codificados
			if (uc < ' ')
				return false;
			result.push_back(str[i]);
		}
	}
	if (!isValidUTF8(result))
		return false;
	return true;
}

// valida regras básicas do URI
bool ParseUri::validateUri(const std::string &uri, std::string &path, std::string &path_info, std::string &query) {
	static const size_t MAX_URI_LEN = 16 * 1024;

	if (uri.empty() || uri[0] != '/')
		return false;
	if (uri.size() > MAX_URI_LEN)
		return false;
	
	if (uri.find('\r') != std::string::npos
		|| uri.find('\n') != std::string::npos
		|| uri.find('\0') != std::string::npos
		|| uri.find('\t') != std::string::npos
		|| uri.find('#') != std::string::npos)
		return false;
	
	for (size_t i = 0; i < uri.size(); ++i) {
		unsigned char c = static_cast<unsigned char>(uri[i]);
		if (c < ' ' && c != '\t')
			return false;
	}
	// separa query após '?'
	size_t query_pos = uri.find('?');
	std::string path_full;
	if (query_pos == std::string::npos) {
		path_full = uri;
		query.clear();
	}
	else {
		path_full = uri.substr(0, query_pos);
		query = uri.substr(query_pos + 1);
	}
	size_t script = std::string::npos;

	const char* extensions[] = {".php", ".cgi", ".py", ".pl", NULL};
	for (int i = 0; extensions[i] != NULL; i++) {
		size_t pos = path_full.find(extensions[i]);
		while (pos != std::string::npos) {
			size_t ext_end = pos + strlen(extensions[i]);
			if (ext_end == path_full.size() || path_full[ext_end] == '/') {
				script = ext_end;
				break;
			}
			pos = path_full.find(extensions[i], pos + 1);
		}
		if (script != std::string::npos)
			break;
	}
	if (script != std::string::npos) {
		path = path_full.substr(0, script);
		if (script < path_full.size())
			path_info = path_full.substr(script);
		else
			path_info.clear();
	}
	else {
		path = path_full;
		path_info.clear();
	}
	return true;
}

// verifica e normaliza os paths 
bool ParseUri::normalizePath(const std::string &raw_path, std::string &normalized_path) {
	if (raw_path.empty() || raw_path[0] != '/')
		return false;
	std::string	decoded_path;
	if (!urlDecodePath(raw_path, decoded_path))
		return false;
	for (size_t i = 0; i < decoded_path.size(); ++i) {
		unsigned char c = static_cast<unsigned char>(decoded_path[i]);
		if (c < ' ' && c != '\t')
			return false;
		if (c == '<' || c == '>' || c == '|' || c == '"' || c == '\\' || c == '\t')
			return false;
	}
	
	std::vector<std::string> stack;
	std::string s = decoded_path;
	size_t i = 1;
	
	while (i <= s.size()) {
		size_t j = s.find('/', i);
		std::string segment;
		if (j == std::string::npos)
			segment = s.substr(i);
		else
			segment = s.substr(i, j - i);
		if (segment.find('\r') != std::string::npos
			|| segment.find('\n') != std::string::npos
			|| segment.find(' ') != std::string::npos)
			return false;
		
		if (segment.empty() || segment == ".") { }
		else if (segment == "..") {
			if (!stack.empty())
				stack.pop_back();
		}
		else
			stack.push_back(segment);
		
		if (j == std::string::npos)
			break;
		i = j + 1;
	}
	
	if (stack.empty())
		normalized_path = "/";
	else {
		normalized_path.clear();
		for (size_t k = 0; k < stack.size(); ++k) {
			normalized_path += '/';
			normalized_path += stack[k];
		}
	}
	return true;
}
