#ifndef PARSEURI_HPP
#define PARSEURI_HPP

#include <string>
#include <cstring>
#include <vector>

class ParseUri {
	private:
		ParseUri(void);
		~ParseUri(void);

	public:
		static int hexDigit(char c);
		static int hexValue(char hi, char lo);
		static bool isValidUTF8(const std::string &s);
		static bool urlDecodePath(const std::string &str, std::string &result);
		static bool urlDecodeQuery(const std::string &str, std::string &result);
		static bool validateUri(const std::string &uri, std::string &path, std::string &path_info, std::string &query);
		static bool normalizePath(const std::string &raw_path, std::string &normalized_path);
};

#endif
