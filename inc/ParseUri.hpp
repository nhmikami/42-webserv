/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseUri.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/24 17:30:00 by marvin            #+#    #+#             */
/*   Updated: 2025/11/24 17:30:00 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSEURI_HPP
#define PARSEURI_HPP

#include <string>
#include <vector>

class ParseUri {
	private:
		ParseUri(void);
		~ParseUri(void);

	public:
		static int hexDigit(char c);
		static int hex_value(char hi, char lo);
		static bool isValidUTF8(const std::string &s);
		static bool urlDecodePath(const std::string &str, std::string &result);
		static bool urlDecodeQuery(const std::string &str, std::string &result);
		static bool validate_uri(const std::string &uri, std::string &path, std::string &query);
		static bool normalizePath(const std::string &raw_path, std::string &normalized_path);
};

#endif
