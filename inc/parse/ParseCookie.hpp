#ifndef PARSECOOKIE_HPP
# define PARSECOOKIE_HPP

# include <string>
# include <map>
# include <sstream>

# include "utils/ParseUtils.hpp"
# include "ParseUri.hpp"

class ParseCookie {
	private:
		ParseCookie();
		~ParseCookie();

	public:
		static const size_t MAX_COOKIE_SIZE = 4096;
		static const size_t MAX_COOKIE_NAME_SIZE = 256;
		static const size_t MAX_COOKIE_VALUE_SIZE = 4096;
		static const size_t MAX_COOKIES_COUNT = 50;

		static bool validateName(const std::string& name);
		static bool validateValue(const std::string& value);
		static bool validateCookieSize(const std::string& cookie_header);
		static std::map<std::string, std::string> parseCookie(const std::string& cookie_header);
		static bool validateCookie(const std::string &cookie);
};

#endif