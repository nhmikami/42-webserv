#include "utils/ParseUtils.hpp"

// ParseUtils::ParseUtils(void) {};

// ParseUtils::ParseUtils(const ParseUtils &other) 
// {
//     (void)other;
// };

// ParseUtils::~ParseUtils(void) {};

// ParseUtils& ParseUtils::operator=(const ParseUtils &other)
// {
//     (void)other;
//     return *this;
// };

bool		ParseUtils::isnumber(const std::string &s)
{
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
	size_t start = s.find_first_not_of(" \t");
	size_t end = s.find_last_not_of(" \t");
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

std::string ParseUtils::replaceChar(std::string str, char find, char replace) {
	for (size_t i = 0; i < str.length(); ++i) {
		if (str[i] == find)
			str[i] = replace;
	}
	return str;
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
