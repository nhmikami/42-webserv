/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseCookie.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 15:17:47 by marvin            #+#    #+#             */
/*   Updated: 2025/11/25 15:17:47 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSECOOKIE_HPP
# define PARSECOOKIE_HPP

# include <string>
# include <map>
# include <sstream>

class ParseCookie {
	private:
		ParseCookie();
		~ParseCookie();

	public:
		static const size_t MAX_COOKIE_SIZE = 4096;
		static const size_t MAX_COOKIE_NAME_SIZE = 256;
		static const size_t MAX_COOKIE_VALUE_SIZE = 4096;
		static const size_t MAX_COOKIES_COUNT = 50;

		static std::map<std::string, std::string> parse_cookie(const std::string& cookie_header);
		static bool validate_name(const std::string& name);
		static bool validate_value(const std::string& value);
		static bool validate_cookie_size(const std::string& cookie_header);
		static std::string buildSetCookie(
			const std::string& name,
			const std::string& value,
			const std::string& path = "/",
			int max_age = -1,
			bool http_only = true,
			bool secure = false,
			const std::string& same_site = "Lax"
		);
};

#endif