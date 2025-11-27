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
		static bool validate_host_header(const std::string &host);
		static bool validate_content_length(const std::string &content_length_str, size_t &out_length);
		static bool validate_transfer_encoding(const std::string &transfer_encoding);
		static bool validate_content_type(const std::string &content_type);
		static bool validate_connection(const std::string &connection);
		static bool validate_accept(const std::string &accept);
		static bool validate_quality_value(const std::string &s);
		static bool validate_type_token(const std::string &t, bool is_type);
		static bool check_params_q(const std::string &params_str, double &out_q, bool &has_q);
		static double q_to_double(const std::string &s);
		static HttpStatus validate_headers(const std::map<std::string, std::string> &headers);
};

#endif
