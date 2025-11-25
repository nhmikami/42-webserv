#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <set>
#include <vector>
#include <string>

struct LocationConfig {
	std::string					_path;
	std::string					_root;
	std::set<std::string>		_methods;
	std::vector<std::string>	_index_files;
	bool						_autoindex;
	size_t						_client_max_body_size;
	std::map<int, std::string>	_error_pages;
	// bool is_cgi=false;
	// std::string cgi_path;
};

#endif