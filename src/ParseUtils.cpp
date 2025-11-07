#include "ParseUtils.hpp"

ParseUtils::ParseUtils(void) {};

ParseUtils::ParseUtils(const ParseUtils &other) {};

ParseUtils::~ParseUtils(void) {};

ParseUtils& ParseUtils::operator=(const ParseUtils &other) {};

void ParseUtils::eraseLine(std::string &buffer, const std::string &text, size_t pos)
{
    size_t  end;

    end = buffer.find("\n", end);
    buffer.erase(pos, end - pos);

};

void ParseUtils::eraseAll(std::string &buffer, const std::string &text)
{
    size_t  pos;

    while (pos != std::string::npos)
    {
        pos = buffer.find(text);
        eraseLine(buffer, text, pos);
    }
};