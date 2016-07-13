#include "tinymq.h"
namespace tinymq
{
	tinyServer* tinyServer::instance()
	{
		if (_instance == NULL)
		{
			_instance = new tinyServer();
		}

		return _instance;
	}
	void tinyServer::deleteSession(const std::string& clientid)
	{
		auto iter = _sessions.find(clientid);
		if (iter != _sessions.end())
		{
			delete iter->second;
			_sessions.erase(iter);
		}
	}
	void tinyServer::reuseSession(const std::string& clientid, tinySocket* sock)
	{
		auto iter = _sessions.find(clientid);
		if (iter != _sessions.end())
		{
			if (iter->second->getClearSession())
			{
				delete iter->second;
				_sessions.erase(iter);
				_sessions.insert(std::make_pair(clientid,new tinySession(sock, false)));
			}
			else {
				iter->second->clearSock();
				iter->second->setSock(sock);
				dynamic_cast<clientEventProcessor*>(sock->getProcessor())->setSession(iter->second);
			}
		}
		else
		{
			_sessions.insert(std::make_pair(clientid, new tinySession(sock, false)));
		}
	}

	void tinyServer::addSession(const std::string& clientid,tinySession* session)
	{
		_sessions.insert(std::make_pair(clientid, session));
	}
	tinyServer* tinyServer::_instance = NULL;
}
