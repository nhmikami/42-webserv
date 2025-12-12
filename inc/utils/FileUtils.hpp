#ifndef FILE_UTILS_HPP
#define FILE_UTILS_HPP

#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

class FileUtils {
	public:
		FileUtils(void);
		~FileUtils(void);

		static bool	exists(const std::string& path);
		static bool	isCGI(const std::string& path);
		static bool	isFile(const std::string& path);
		static bool	isDirectory(const std::string& path);
		static bool	isReadable(const std::string& path);
		static bool	isWritable(const std::string& path);
		static bool	isExecutable(const std::string& path);

		static std::string	resolvePath(const std::string &root, const std::string &path);
		static std::string	normalizePath(const std::string &path);
		static std::string	guessMimeType(const std::string &path);

};

#endif