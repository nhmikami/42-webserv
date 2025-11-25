#include "ParseConfig.hpp"

ParseConfig::ParseConfig(void) : _filename(""), _context(GLOBAL), _open_brackets(false) {};

ParseConfig::ParseConfig(const ParseConfig &other) : _filename(other._filename), _context(GLOBAL), _open_brackets(false) {};

ParseConfig::ParseConfig(const std::string &filename) : _filename(filename), _context(GLOBAL), _open_brackets(false) {};

ParseConfig::~ParseConfig(void) {};

ParseConfig &ParseConfig::operator=(const ParseConfig &other) {
	(void)other;
	return *this;
};

std::vector<ServerConfig> ParseConfig::parse() {

	std::ifstream file(_filename.c_str());

	if (!file.is_open())
		throw std::invalid_argument("Unable to open file: " + _filename);;

	std::string line;

	while(std::getline(file, line)) {
		size_t pos = line.find('#');
		if (pos != std::string::npos) {
			line = line.substr(0, pos);
		}
		line = ParseUtils::trim(line);
		if (line.length() > 0 && !line.empty()) {
			if (changeContext(line))
				continue;
			parseLine(line);
		}
	}

	file.close();
	if (_open_brackets != 0) {
		throw std::invalid_argument("Sintax error in file " + _filename + ": Open brackets.");
	}
	return _servers;
}

bool ParseConfig::changeContext(std::string line) {
	std::istringstream iss(line);
	std::string key;
	std::string error_message;

	iss >> key;
	error_message = "Sintax error in file " + _filename + "near " + key;

	if (line.find("{") != std::string::npos && line.find('}') == std::string::npos){
		_open_brackets++;
		if (key == "server" || key == "location") {
			_context = static_cast<Context>(_context + 1);
			if (key == "server")
                _servers.push_back(ServerConfig());
            else if (key == "location")
				_servers.back().addLocation();
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

void ParseConfig::parseLine(std::string line){
	std::istringstream iss(line);
	std::string key;
	std::vector<std::string> values;
	std::string token;

	if (line.empty() || line[line.length() - 1] != ';') {
        throw std::invalid_argument("Syntax error in file " + _filename + ": Missing semicolon.");
    }

	iss >> key;

	while (iss >> token)
	{
		if (!token.empty() && token[token.length() - 1] == ';')
			token = token.substr(0, token.length() - 1);
		if (!token.empty())
			values.push_back(token);
	}

	if (_context == SERVER) {
		_servers.back().parseServer(key, values);
	} else if (_context == LOCATION) {
		_servers.back().parseLocation(key, values);
	}
}
