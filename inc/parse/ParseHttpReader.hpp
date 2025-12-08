#ifndef PARSEHTTPREADER_HPP
#define PARSEHTTPREADER_HPP

#include <string>
#include <sys/socket.h>
#include <poll.h>
#include <cerrno>
#include <cstdlib>
#include <stdint.h>
#include <limits.h>

#include "../Response.hpp"
#include "parse/ParseHttp.hpp"
#include "parse/ParseHttpValidator.hpp"
#include "utils/ParseUtils.hpp"

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
		
		static HttpStatus validateBodyChunked(
			size_t max_body_size,
			std::string &buffer,
			std::string &out_body);
			static bool hexToSize(const std::string &hex_str, size_t &out_size);
};

#endif
