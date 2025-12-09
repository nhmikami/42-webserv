#include "config/ParseConfig.hpp"

// ParseConfig::ParseConfig(void) : _filename(""), _context(GLOBAL), _open_brackets(0), _count_line(0) {};

// ParseConfig::ParseConfig(const ParseConfig &other) : _filename(other._filename), _context(GLOBAL), _open_brackets(0), _count_line(0) {};

ParseConfig::ParseConfig(const std::string &filename) : _filename(filename), _context(GLOBAL), _open_brackets(0), _count_line(0) {};

ParseConfig::~ParseConfig(void) {};

// ParseConfig &ParseConfig::operator=(const ParseConfig &other) {
// 	(void)other;
// 	return *this;
// };

std::vector<ServerConfig> ParseConfig::parse(void) {

	std::ifstream file(_filename.c_str());

	if (!file.is_open())
		throw std::invalid_argument("Unable to open file: " + _filename);

	if (file.peek() == std::ifstream::traits_type::eof())
		throw std::invalid_argument("Empty configuration file: " + _filename);

	std::string line;
	
	while(std::getline(file, line)) {
		std::string key;
		std::vector<std::string> values;
		size_t pos = line.find('#');
		_count_line++;

		if (pos != std::string::npos) {
			line = line.substr(0, pos);
		}
		line = ParseUtils::trim(line);

		if (!getKeyValues(line, &key, &values))
			continue;

		if (!line.empty()) {
			if (changeContext(key, values))
				continue;
			parseLine(key, values);
		}
	}

	file.close();

	if (_open_brackets != 0) {
		throw std::invalid_argument("Syntax error in file " + _filename + " line " + ParseUtils::itoa(_count_line) + ": Open brackets.");
	}

	return _servers;
}

bool ParseConfig::getKeyValues(const std::string line, std::string *key, std::vector<std::string> *values)
{
	std::istringstream iss(line);
	std::string token;

	if (line.empty())
		return false;

	iss >> *key;

	if (*key != "server" && *key != "location" && *key != "}" && line[line.length() - 1] != ';') {
		throw std::invalid_argument("Syntax error in file " + _filename + " line " + ParseUtils::itoa(_count_line) + ": Missing semicolon.");
	}

	while (iss >> token)
	{
		if (!token.empty() && token[token.length() - 1] == ';')
			token = token.substr(0, token.length() - 1);
		if (!token.empty())
			values->push_back(token);
	}

	return true;
}

bool ParseConfig::changeContext(const std::string key, std::vector<std::string> values) {
	std::string error_message;

	error_message = "Syntax error in file " + _filename + " line " + ParseUtils::itoa(_count_line) + " near " + key;

	std::string open_bracket = "{";
	std::string close_bracket = "}";
	std::vector<std::string>::iterator it_open_brackets = std::find(values.begin(), values.end(), open_bracket);
	std::vector<std::string>::iterator it_close_brackets = std::find(values.begin(), values.end(), close_bracket);

	if (it_open_brackets != values.end() && it_close_brackets == values.end()){
		_open_brackets++;
		if (key == "server" || key == "location") {
			_context = static_cast<Context>(_context + 1);
			if (key == "server")
				_servers.push_back(ServerConfig());
			else if (key == "location")
				_servers.back().addLocation(values, &_location_path);
			return true;
		} else
			throw std::invalid_argument(error_message);
	} else {
		if (key == "server" || key == "location")
			throw std::invalid_argument(error_message);
		else if (key == "}") {
			if (_context == GLOBAL)
				throw std::invalid_argument(error_message);
			_open_brackets--;
			_context = static_cast<Context>(_context - 1);
			return true;
		}
	}
	return false;
}

void ParseConfig::parseLine(const std::string key, std::vector<std::string> values){
	if (_context == SERVER) {
		_servers.back().parseServer(key, values);
	} else if (_context == LOCATION) {
		LocationConfig* loc = _servers.back().getLocation(_location_path);
		if (!loc) {
			Logger::log(Logger::WARNING, "Failed to parse location: " + _location_path);
			return ;
		}
		loc->parseLocation(key, values);
	}
}
