#include "parse/ParseHttpReader.hpp"

ParseHttpReader::ParseHttpReader(void) {}

ParseHttpReader::~ParseHttpReader(void) {}

HttpStatus ParseHttpReader::validateBodyContentLength(
	const std::string &content_length_header,
	size_t max_body_size,
	std::string &buffer,
	std::string &out_body)
{
	size_t content_length;
	if (!ParseHttpValidator::validateContentLength(content_length_header, content_length))
		return BAD_REQUEST;
	
	if (content_length > max_body_size)
		return PAYLOAD_TOO_LARGE;
	
	if (buffer.size() < content_length)
		return CONTINUE;
	
	out_body = buffer.substr(0, content_length);
	buffer = buffer.substr(content_length);
	
	return OK;
}

bool ParseHttpReader::hexToSize(const std::string &hex_str, size_t &out_size) {
	if (hex_str.empty())
		return false;
	
	std::string hex_clean = ParseUtils::trim(hex_str);
	
	size_t semicolon_pos = hex_clean.find(';');
	if (semicolon_pos != std::string::npos)
		hex_clean = hex_clean.substr(0, semicolon_pos);
	
	hex_clean = ParseUtils::trim(hex_clean);
	if (hex_clean.empty())
		return false;
	
	out_size = 0;
	for (size_t i = 0; i < hex_clean.size(); i++) {
		char c = hex_clean[i];
		int digit;
		
		if (c >= '0' && c <= '9')
			digit = c - '0';
		else if (c >= 'a' && c <= 'f')
			digit = c - 'a' + 10;
		else if (c >= 'A' && c <= 'F')
			digit = c - 'A' + 10;
		else
			return false;

		size_t sdigit = static_cast<size_t>(digit);
		if (out_size > (std::numeric_limits<size_t>::max() - sdigit) / 16)
			return false;
		
		out_size = (out_size * 16) + sdigit;
	}
	return true;
}

bool ParseHttpReader::isLastTokenChunked(const std::string &transfer_encoding) {
	if (transfer_encoding.empty())
		return false;
	
	std::string te_trim = ParseUtils::trim(transfer_encoding);
	if (te_trim.empty())
		return false;
	
	size_t last_comma = te_trim.rfind(',');
	std::string last_token;
	
	if (last_comma == std::string::npos)
		last_token = te_trim;
	else
		last_token = te_trim.substr(last_comma + 1);
	
	last_token = ParseUtils::trim(last_token);
	return (last_token == "chunked");
}

HttpStatus ParseHttpReader::validateBodyChunked(
	size_t max_body_size,
	std::string &buffer,
	std::string &out_body)
{
	size_t total_body_size = out_body.size();
	
	while (true) {
		size_t crlf_pos = buffer.find("\r\n");
		if (crlf_pos == std::string::npos)
			return CONTINUE;
		
		std::string size_line = buffer.substr(0, crlf_pos);
		size_t chunk_size;
		if (!hexToSize(size_line, chunk_size))
			return BAD_REQUEST;
		
		buffer = buffer.substr(crlf_pos + 2);
		
		if (chunk_size == 0) {
			size_t max_trailers = 100;
			size_t trailer_count = 0;
			while (true) {
				size_t crlf_pos = buffer.find("\r\n");
				if (crlf_pos == std::string::npos)
					return CONTINUE;
				if (crlf_pos == 0) {
					buffer = buffer.substr(2);
					return OK;
				}
				buffer = buffer.substr(crlf_pos + 2);
				
				if (++trailer_count > max_trailers)
					return BAD_REQUEST;
			}
		}

		if (chunk_size > max_body_size - total_body_size)
			return PAYLOAD_TOO_LARGE;
		
		if (buffer.size() < chunk_size + 2)
			return CONTINUE;
			if (buffer[chunk_size] != '\r' || buffer[chunk_size + 1] != '\n')
		
			return BAD_REQUEST;
		
		out_body.append(buffer.substr(0, chunk_size));
		total_body_size += chunk_size;
		
		buffer = buffer.substr(chunk_size + 2);
	}
}