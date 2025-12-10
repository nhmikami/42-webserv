#ifndef PARSEHTTPREADER_HPP
# define PARSEHTTPREADER_HPP

# include <string>
# include <cstdlib>
# include <limits>

# include "../Response.hpp"
# include "utils/ParseUtils.hpp"
# include "ParseHttpValidator.hpp"

class ParseHttpReader {
	private:
		ParseHttpReader(void);
		~ParseHttpReader(void);

	public:
		static HttpStatus validateBodyContentLength(
			const std::string &content_length_header,
			size_t max_body_size,
			std::string &buffer,
			std::string &out_body);
		static bool hexToSize(const std::string &hex_str, size_t &out_size);
		static bool isLastTokenChunked(const std::string &transfer_encoding);
		static HttpStatus validateBodyChunked(
			size_t max_body_size,
			std::string &buffer,
			std::string &out_body);
};

#endif
