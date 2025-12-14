#ifndef SESSION_HPP
#define SESSION_HPP

#include <string>
#include <map>
#include <ctime>

class Session {
private:
	std::string							_id;
	time_t								_lastAccess;
	std::map<std::string, std::string>	_data;

public:
	Session(const std::string& id);
	~Session(void);

	const std::string&	getId(void) const;

	void		touch(void);
	bool		isExpired(time_t now, time_t timeout) const;

	void		set(const std::string& key, const std::string& value);
	bool		has(const std::string& key) const;
	std::string	get(const std::string& key) const;
};

#endif