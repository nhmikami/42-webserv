/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseHttpValidator.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/24 17:30:00 by marvin            #+#    #+#             */
/*   Updated: 2025/11/24 17:30:00 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSEHTTPVALIDATOR_HPP
#define PARSEHTTPVALIDATOR_HPP

#include <string>
#include <map>
#include <cerrno>
#include <cstdlib>
#include <cctype>
#include "Response.hpp"

class ParseHttpValidator {
	private:
		ParseHttpValidator(void);
		~ParseHttpValidator(void);

	public:
		static bool validateHostHeader(const std::string &host);
		static bool validateContentLength(const std::string &content_length_str, size_t &out_length);
		static bool validateTransferEncoding(const std::string &transfer_encoding);
		static bool validateContentType(const std::string &content_type);
		static bool validateConnection(const std::string &connection);
		static bool validateAccept(const std::string &accept);
		static bool validateQualityValue(const std::string &s);
		static bool validateTypeToken(const std::string &t, bool is_type);
		static bool checkParamsQ(const std::string &params_str, double &out_q, bool &has_q);
		static double qToDouble(const std::string &s);
		static HttpStatus validateHeaders(const std::map<std::string, std::string> &headers);
};

#endif
