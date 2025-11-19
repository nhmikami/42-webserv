#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>

class Logger {

	private:
		Logger(const Logger &other);

		Logger &operator=(const Logger &other);

	public:
		enum LogLevel {
			INFO,
			WARNING,
			ERROR,
			SERVER,
			MAX_LEVEL
		};

		Logger(void);
		~Logger(void);

		void 		log(LogLevel level, const std::string &msg);
		std::string	getLevel(LogLevel level);
};

#endif 