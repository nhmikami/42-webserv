#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

struct LocationConfig {
	std::string					path;
	std::string					root;
	std::set<std::string>		methods;
	std::vector<std::string>	index_files;
	bool						autoindex = false;
	size_t						client_max_body_size = DEFAULT;
	// bool is_cgi=false;
	// std::string cgi_path;
};

#endif