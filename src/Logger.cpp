#include "Logger.hpp"

// Logger::Logger(void){};

// Logger::Logger(const Logger &other){
// 	(void)other;
// };

// Logger::~Logger(void){};

// Logger &Logger::operator=(const Logger &other){
// 	(void)other;
// 	return *this;
// };

void Logger::log(LogLevel level, const std::string &msg){
	std::cout << getLevel(level) << msg << std::endl;
};

std::string Logger::getLevel(LogLevel level){
	static const std::string levels[] = {
		"\033[1;36m[INFO]\033[0m ",
        "\033[1;33m[WARNING]\033[0m ",
        "\033[1;31m[ERROR]\033[0m ",
        "\033[1;34m[SERVER]\033[0m "
	};

	if (level < MAX_LEVEL)
		return levels[level];
	return "[DEFAULT]";
}