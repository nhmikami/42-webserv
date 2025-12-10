#ifndef PARSEHTTPVALIDATOR_HPP
# define PARSEHTTPVALIDATOR_HPP

# include <string>
# include <map>
# include <vector>
# include <cerrno>
# include <cstdlib>
# include <cctype>

# include "http/Response.hpp"
# include "ParseCookie.hpp"
# include "utils/ParseUtils.hpp"

class ParseHttpValidator {
	private:
		ParseHttpValidator(void);
		~ParseHttpValidator(void);

	public:
		static bool validateHostHeader(const std::string &host);
		static bool validateUserAgent(const std::string &user_agent);
		static bool validateContentLength(const std::string &content_length_str, size_t &out_length);
		static bool validateTransferEncoding(const std::string &transfer_encoding);
		static bool validateContentType(const std::string &content_type);
		static bool validateConnection(const std::string &connection);
		static bool validateAccept(const std::string &accept);
		static HttpStatus validateHeaders(const std::map<std::string, std::string> &headers);

	private:
		static bool validateQualityValue(const std::string &s);
		static double qToDouble(const std::string &s);
		static bool validateTypeToken(const std::string &t);
		static bool validateParamsQ(const std::string &params_str);
};

#endif
