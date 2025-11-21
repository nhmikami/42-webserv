#include "ParseConfig.hpp"

ParseConfig::ParseConfig(void) : _filename("") {};

ParseConfig::ParseConfig(const ParseConfig &other) : _filename(other._filename) {};

ParseConfig::ParseConfig(const std::string &filename) : _filename(filename) {};

ParseConfig::~ParseConfig(void) {};

ParseConfig &ParseConfig::operator=(const ParseConfig &other) {
    (void)other;
    return *this;
};

Config ParseConfig::parse() {

    std::ifstream file(_filename.c_str());

    if (!file.is_open())
        throw std::invalid_argument("Unable to open file: " + _filename);

    std::string line;
    size_t count_lines = 1;

    while(std::getline(file, line)) {
        size_t pos = line.find('#');
        if (pos != std::string::npos) {
            line = line.substr(0, pos);
        }
        line = ParseUtils::trim(line);
        if (line.length() > 0 && !line.empty()) {
            parseLine(line);
        }
        
        count_lines++;
    }
    file.close();
    return _config;
}

void ParseConfig::parseLine(std::string line){
    size_t i = 0;

    while (i < line.size()){
        char c = line[i];

        if (i == 0 && std::isalpha(c))
            parseKey(line, i);
        else if (std::isdigit(c))
            parseNumber(line, i);
        else if (std::isalpha(c))
            parseString(line, i);
        else if (std::)
    }
}
