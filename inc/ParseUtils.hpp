#ifndef PARSE_UTILS
#define PARSE_UTILS

#include <string>

class ParseUtils {
    private:
        ParseUtils(const ParseUtils &other);
        ParseUtils& operator=(const ParseUtils &other);

    public:
        ParseUtils(void);
        ~ParseUtils(void);

        void eraseLine(std::string &buffer, const std::string &text, size_t pos);
        void eraseAll(std::string &buffer, const std::string &text);

};

#endif