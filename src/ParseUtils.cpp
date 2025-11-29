#include "ParseUtils.hpp"

ParseUtils::ParseUtils(void) {};

ParseUtils::ParseUtils(const ParseUtils &other) 
{
    (void)other;
};

ParseUtils::~ParseUtils(void) {};

ParseUtils& ParseUtils::operator=(const ParseUtils &other)
{
    (void)other;
    return *this;
};

// void ParseUtils::eraseLine(std::string &buffer, const std::string &text, size_t pos)
// {
//     size_t  end = size(buffer);

//     end = buffer.find("\n", end);
//     buffer.erase(pos, end - pos);

// };

// void ParseUtils::eraseAll(std::string &buffer, const std::string &text)
// {
//     size_t  pos = 0;

//     while (pos != std::string::npos)
//     {
//         pos = buffer.find(text);
//         eraseLine(buffer, text, pos);
//     }
// };

std::string ParseUtils::itoa(int n)
{
    std::stringstream ss;
    ss << n;
    return ss.str();
};

std::string ParseUtils::trim(const std::string &s) {
	size_t start = s.find_first_not_of(" \t");
	size_t end = s.find_last_not_of(" \t");
	if (start == std::string::npos || end == std::string::npos)
		return "";
	return s.substr(start, end - start + 1);
}
