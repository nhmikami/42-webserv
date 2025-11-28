#ifndef PARSEUTILS_HPP
#define PARSEUTILS_HPP

#include <string>
#include <sstream>
#include <iostream>

class ParseUtils {
    private:
        ParseUtils(const ParseUtils &other);
        ParseUtils& operator=(const ParseUtils &other);

    public:
        ParseUtils(void);
        ~ParseUtils(void);

        static std::string itoa(int n);
        static std::string trim(const std::string &s);
};

#endif