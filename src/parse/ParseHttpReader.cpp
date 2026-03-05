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
		return HTTP_PENDING;
	
	out_body = buffer.substr(0, content_length);
	buffer = buffer.substr(content_length);
	
	return OK;
}

bool ParseHttpReader::hexToSizeRange(const std::string &str, size_t start, size_t end, size_t &out) {
	if (start == end)
		return false;
	
	out = 0;
	for (size_t i = start; i < end; ++i) {
		char c = str[i];
		if (c == ';') break;
		
		int digit = -1;
		if (c >= '0' && c <= '9') digit = c - '0';
		else if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;
		else if (c >= 'A' && c <= 'F') digit = c - 'A' + 10;
		else if (isspace(c)) continue;
		else return false;

		if (digit != -1) {
			if (out > (std::numeric_limits<size_t>::max() >> 4)) return false;
			out = (out << 4) | digit;
		}
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

HttpStatus ParseHttpReader::validateBodyChunked(size_t max_body_size, std::string &buffer, std::string &out_body) {
	size_t total_body_size = out_body.size();
	size_t processed_pos = 0;

	if (out_body.capacity() < max_body_size)
		out_body.reserve(max_body_size);

	while (true) {
		size_t crlf_pos = buffer.find("\r\n", processed_pos);
		if (crlf_pos == std::string::npos)
			break;

		size_t chunk_size = 0;
		if (!hexToSizeRange(buffer, processed_pos, crlf_pos, chunk_size))
			return BAD_REQUEST;

		if (chunk_size == 0) {
			size_t final_crlf = buffer.find("\r\n", crlf_pos + 2);
			if (final_crlf == std::string::npos)
				break;
			
			processed_pos = final_crlf + 2;
			buffer.erase(0, processed_pos);
			return OK;
		}

		size_t chunk_data_start = crlf_pos + 2;
		if (buffer.size() < chunk_data_start + chunk_size + 2)
			break;

		if (chunk_size > max_body_size - total_body_size)
			return PAYLOAD_TOO_LARGE;
		
		if (buffer[chunk_data_start + chunk_size] != '\r' || buffer[chunk_data_start + chunk_size + 1] != '\n')
			return BAD_REQUEST;

		out_body.append(buffer, chunk_data_start, chunk_size);
		total_body_size += chunk_size;
		processed_pos = chunk_data_start + chunk_size + 2;
	}

	if (processed_pos > 0) {
		buffer.erase(0, processed_pos);
	}
	return HTTP_PENDING;
}
