#include "utils/ParseUtils.hpp"

bool		ParseUtils::isNumber(const std::string &s) {
	if (s.empty())
		return false;

	size_t i = 0;
	if ((s[0] == '-' || s[0] == '+') && std::isdigit(s[1]))
		i++;
	for (; i < s.size(); ++i)
	{
		if (!std::isdigit(s[i]))
			return false;
	}
	return true;
}

bool		ParseUtils::isUnsigNumber(const std::string &s) {
	if (s.empty())
		return false;

	for (size_t i = 0; i < s.size(); ++i)
	{
		if (!std::isdigit(s[i]))
			return false;
	}
	return true;
}

std::string ParseUtils::itoa(int n)
{
	std::stringstream ss;
	ss << n;
	return ss.str();
};

std::string ParseUtils::trim(const std::string &s)
{
	const std::string whitespace = " \t\r\n";
	size_t start = s.find_first_not_of(whitespace);
	size_t end = s.find_last_not_of(whitespace);
	if (start == std::string::npos || end == std::string::npos)
		return "";
	return s.substr(start, end - start + 1);
};

std::string ParseUtils::toUpper(std::string str) {
	for (size_t i = 0; i < str.length(); ++i) {
		str[i] = std::toupper(static_cast<unsigned char>(str[i]));
	}
	return str;
}

std::string ParseUtils::toLower(std::string str) {
	for (size_t i = 0; i < str.length(); ++i) {
		str[i] = std::tolower(static_cast<unsigned char>(str[i]));
	}
	return str;
}

std::string ParseUtils::replaceChar(std::string str, char find, char replace) {
	for (size_t i = 0; i < str.length(); ++i) {
		if (str[i] == find)
			str[i] = replace;
	}
	return str;
}

std::vector<std::string> ParseUtils::split(const std::string &str, char delimiter) {
	std::vector<std::string> tokens;
	std::string token;
	std::stringstream ss(str);

	while (std::getline(ss, token, delimiter))
		tokens.push_back(token);

	return tokens;
}

bool ParseUtils::hasSpecialChar(const std::string &str)
{
	for (size_t i = 0; i < str.length(); i++)
	{
		char c = str[i];
		if (!std::isalnum(c) && c != '.' && c != '-' && c != '_')
			return true;
	}
	return false;
}

std::string ParseUtils::htmlEscape(const std::string &s) {
	std::string escaped;
	for (size_t i = 0; i < s.length(); ++i) {
		switch (s[i]) {
			case '&':
				escaped += "&amp;";
				break ;
			case '<':
				escaped += "&lt;";
				break ;
			case '>':
				escaped += "&gt;";
				break ;
			case '"':
				escaped += "&quot;";
				break ;
			case '\'':
				escaped += "&#39;";
				break ;
			default:
				escaped += s[i];
		}
	}
	return escaped;
}

std::pair<std::string, std::string> ParseUtils::splitHeadersAndBody(const std::string& raw) {
	size_t headerEnd = raw.find("\r\n\r\n");
	size_t sepLen = 4;

	if (headerEnd == std::string::npos) {
		headerEnd = raw.find("\n\n");
		sepLen = 2;
	}

	if (headerEnd == std::string::npos)
		return std::make_pair("", raw);

	return std::make_pair(raw.substr(0, headerEnd), raw.substr(headerEnd + sepLen));
}

std::pair<std::string, std::string> ParseUtils::splitPair(const std::string& s, const std::string& delimiter) {
	size_t pos = s.find(delimiter);
	if (pos == std::string::npos)
		return std::make_pair(s, "");
	return std::make_pair(s.substr(0, pos), s.substr(pos + delimiter.length()));
}

std::string ParseUtils::extractAttribute(const std::string& header, const std::string& key) {
	std::string target = key + "=";
	size_t pos = 0;
	
	while (true) {
		pos = header.find(target, pos);
		if (pos == std::string::npos)
			return "";
		if (pos == 0 || header[pos - 1] == ';' || header[pos - 1] == ' ' || header[pos - 1] == '\t') {
			break ;
		}
		pos++; 
	}
	pos += target.length();
	if (pos < header.length() && header[pos] == '"') {
		size_t end = header.find('"', pos + 1);
		if (end != std::string::npos)
			return header.substr(pos + 1, end - pos - 1);
	} else {
		size_t end = header.find_first_of("; \r\n", pos);
		if (end == std::string::npos)
			end = header.length();
		return header.substr(pos, end - pos);
	}
	return "";
}
