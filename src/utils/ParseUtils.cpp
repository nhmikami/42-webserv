#include "../../inc/utils/ParseUtils.hpp"

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

bool		ParseUtils::isnumber(const std::string &s)
{
	if (s.empty())
    	return false;

    size_t i = 0;
    if (s[0] == '-' || s[0] == '+')
        i++;
    for (; i < s.size(); ++i)
    {
        if (!std::isdigit(s[i]))
            return false;
    }
	return true;
}

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
