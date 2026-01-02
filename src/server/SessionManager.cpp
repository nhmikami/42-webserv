#include "server/SessionManager.hpp"

SessionManager::SessionManager(time_t timeout) : _timeout(timeout) {
}

SessionManager::~SessionManager(void) {
	for (std::map<std::string, Session*>::iterator it = _sessions.begin(); it != _sessions.end(); ++it)
		delete it->second;
}

std::string SessionManager::_generateSessionId(void) {
	std::ostringstream oss;
	oss << time(NULL) << "_" << rand();
	return oss.str();
}

Session* SessionManager::createSession(void) {
	std::string id;
	do {
		id = _generateSessionId();
	} while (_sessions.count(id) > 0); 
	
	Session* s = new Session(id);
	_sessions[id] = s;
	return s;
}

Session* SessionManager::getSession(const std::string& id) {
	std::map<std::string, Session*>::iterator it = _sessions.find(id);
	if (it == _sessions.end())
		return NULL;

	if (it->second->isExpired(std::time(NULL), _timeout)) {
		delete it->second;
		_sessions.erase(it);
		return NULL;
	}

	it->second->touch();
	return it->second;
}

void SessionManager::cleanup(void) {
	time_t now = std::time(NULL);
	for (std::map<std::string, Session*>::iterator it = _sessions.begin(); it != _sessions.end(); ) {
		if (it->second->isExpired(now, _timeout)) {
			delete it->second;
			_sessions.erase(it++);
		} else {
			++it;
		}
	}
}
