#ifndef PARSEUTILS_HPP
#define PARSEUTILS_HPP

#include <string>
#include <vector>
#include <sstream>
#include <iostream>

#include "Logger.hpp"

class ParseUtils {
	private:
		ParseUtils(const ParseUtils &other);
		ParseUtils& operator=(const ParseUtils &other);

	public:
		ParseUtils(void);
		~ParseUtils(void);

		static std::string  itoa(int n);
		static std::string  trim(const std::string &s);
		static bool			isNumber(const std::string &s);
		static bool			isUnsigNumber(const std::string &s);
		static std::string  toUpper(std::string str);
		static std::string  replaceChar(std::string str, char find, char replace);
		static bool			hasSpecialChar(const std::string &str);
		static std::string  htmlEscape(const std::string &s);

		static std::vector<std::string> split(const std::string &str, char delimiter);
};

#endif