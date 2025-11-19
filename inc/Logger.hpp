#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>

class Logger {
	public:
		enum LogLevel {
			INFO,
			WARNING,
			ERROR,
			SERVER,
		};

		Logger(void);
		Logger(const Logger &other);
		~Logger(void);

		Logger &operator=(const Logger &other);

		void log(LogLevel level, const std::string &msg);
		std::string getLevel(LogLevel level);

};

#endif 