#include "server/Session.hpp"

Session::Session(const std::string& id)	: _id(id), _lastAccess(std::time(NULL)) {
}

Session::~Session(void) {
}

const std::string& Session::getId() const { 
	return _id;
}

void Session::touch() {
	_lastAccess = std::time(NULL);
}

bool Session::isExpired(time_t now, time_t timeout) const {
	return (now - _lastAccess) > timeout;
}

void Session::set(const std::string& key, const std::string& value) {
	_data[key] = value;
}

bool Session::has(const std::string& key) const {
	return _data.count(key) != 0;
}

std::string Session::get(const std::string& key) const {
	std::map<std::string, std::string>::const_iterator it = _data.find(key);
	if (it != _data.end())
		return it->second;
	return "";
}
