#ifndef SESSION_MANAGER_HPP
#define SESSION_MANAGER_HPP

#include "Session.hpp"
#include <map>
#include <cstdlib>
#include <sstream>

class SessionManager {
private:
	time_t							_timeout;
	std::map<std::string, Session*>	_sessions;

	std::string	_generateSessionId();

public:
	SessionManager(time_t timeout);
	~SessionManager(void);

	Session*	createSession(void);
	Session*	getSession(const std::string& sessionId);
	void		cleanup(void);
};

#endif