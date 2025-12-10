#include "parse/ParseCookie.hpp"

ParseCookie::ParseCookie() { }

ParseCookie::~ParseCookie() { }

bool ParseCookie::validateName(const std::string& name) {
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

bool ParseCookie::validateValue(const std::string& value) {
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

bool ParseCookie::validateCookieSize(const std::string& cookie_header) {
	return cookie_header.size() <= MAX_COOKIE_SIZE;
}

std::map<std::string, std::string> ParseCookie::parseCookie(const std::string& cookie_header) {
	std::map<std::string, std::string> cookies;
	if (!validateCookieSize(cookie_header))
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
		if (!validateName(key))
			continue;
		if (!validateValue(value))
			continue;
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