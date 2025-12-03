#include "config/BaseConfig.hpp"

BaseConfig::BaseConfig(void) : 
    _autoindex(false), 
    _client_max_body_size(1000), 
    _is_cgi(false)
{};

BaseConfig::BaseConfig(const BaseConfig &other) :
    _root(other._root),
    _autoindex(other._autoindex),
    _client_max_body_size(other._client_max_body_size),
    _index_files(other._index_files),
    _error_pages(other._error_pages),
    _is_cgi(other._is_cgi),
    _cgi(other._cgi),
	_upload(other._upload)
{};

BaseConfig::~BaseConfig(void) {};

void BaseConfig::setRoot(const std::vector<std::string>& values)
{
	if (values.size() != 1)
		throw std::invalid_argument("root must have exactly one value.");
	const std::string &path = values[0];
	struct stat st;
	if (stat(path.c_str(), &st) != 0 || !S_ISDIR(st.st_mode))
		throw std::invalid_argument("root directory does not exist or is not a directory: " + path);
	_root = path;
};

void BaseConfig::setAutoIndex(const std::vector<std::string>& values)
{
	if (values.size() != 1)
		throw std::invalid_argument("autoindex must have exactly one value.");
	_autoindex = values[0] == "on" 
		? true 
		: values[0] == "off"
		? false
		: throw std::invalid_argument("autoindex invalid value.");
};

void BaseConfig::setClientMaxBodySize(const std::vector<std::string>& values)
{
	if (values.size() != 1)
		throw std::invalid_argument("client_max_body_size must have exactly one value.");
	if (!ParseUtils::isnumber(values[0]))
		throw std::invalid_argument("client_max_body_size must be a number.");
	int client_max_body_size = std::atoi(values[0].c_str());
	if (client_max_body_size < 1)
		throw std::invalid_argument("client_max_body_size must be a positive number.");
	_client_max_body_size = client_max_body_size;
};

void BaseConfig::setIndexFiles(const std::vector<std::string>& values)
{
	if (values.empty())
		throw std::invalid_argument("index must have at least one value.");
	_index_files = values;
};

void BaseConfig::setErrorPages(const std::vector<std::string>& values)
{
	if (values.empty() || values.size() < 2)
		throw std::invalid_argument("error_page must have at least two values (error number and error file).");

	std::string path = values[values.size() - 1];
	for (size_t i = 0; i < values.size() - 1; i++) {
		if (!ParseUtils::isnumber(values[i]))
			throw std::invalid_argument("error code " + values[i] + " is not a number.");
		int error_code = std::atoi(values[i].c_str());
		if (error_code < 100 || error_code > 599)
			throw std::invalid_argument("invalid error code number (must be between 100 and 599).");
		_error_pages[error_code] = path;
	}
};

void BaseConfig::setCgi(const std::vector<std::string>& values)
{
	if (values.size() != 2)
		throw std::invalid_argument("cgi must have two values: extension and root.");
	_is_cgi = true;
	_cgi[values[0]] = values[1];
};

void BaseConfig::setUpload(const std::vector<std::string>& values)
{
	if (values.size() != 1)
		throw std::invalid_argument("upload must have exactly one value.");
	const std::string &path = values[0];
	struct stat st;
	if (stat(path.c_str(), &st) != 0 || !S_ISDIR(st.st_mode))
		throw std::invalid_argument("upload directory does not exist or is not a directory: " + path);
	_upload = path;
};


const std::string 							BaseConfig::getRoot(void) const { return _root; };

const std::vector<std::string>				BaseConfig::getIndexFiles(void) const { return _index_files; };

bool										BaseConfig::getAutoIndex(void) const { return _autoindex; };

size_t										BaseConfig::getClientMaxBodySize(void) const { return _client_max_body_size; };

const std::map<int, std::string>			BaseConfig::getErrorPages(void) const { return _error_pages; };

const std::map<std::string, std::string>	BaseConfig::getCgi(void) const { return _cgi; };

const std::string							BaseConfig::getUpload(void) const { return _upload; };