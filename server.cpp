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
	void tinyServer::eraseSession(const std::string& clientid)
	{
		auto iter = _sessions.find(clientid);
		if (iter != _sessions.end())
		{
			_sessions.erase(iter);
		}
	}
	bool tinyServer::reuseSession(const std::string& clientid, tinySocket* sock)
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
				return true;
			}
		}
		else
		{
			_sessions.insert(std::make_pair(clientid, new tinySession(sock, false)));
		}
		return false;
	}

	void tinyServer::addSession(const std::string& clientid,tinySession* session)
	{
		_sessions.insert(std::make_pair(clientid, session));
	}

	void tinyServer::addWaittingSock(clientEventProcessor * cep, time_t t)
	{
		_waittingConnection.insert(std::make_pair(cep, t));
	}

	void tinyServer::removeWaittingSock(clientEventProcessor * cep)
	{
		_waittingConnection.erase(cep);
	}
	bool tinyServer::addTopicSubscriber(const std::string& topic, clientEventProcessor* cep)
	{

		_addTopic("#", cep);
		size_t last = 0;
		size_t index = topic.find_first_of("/", last);
		std::string resultStr;
		std::string tmp_str = topic.substr(last, index - last);
		if (tmp_str.compare("") == 0)
		{
			last = index + 1;
			index = topic.find_first_of("/", last);
			resultStr.append("/").append(topic.substr(last, index - last));
			_addTopic(resultStr + "/#",cep);
		}
		else {
			resultStr = tmp_str;
			_addTopic(resultStr + "/#", cep);
		}

 		while (index != -1)
		{
			last = index + 1;
			index = topic.find_first_of("/", last);
			std::string tmp_str = topic.substr(last, index - last);
			resultStr.append("/").append(tmp_str);
			if (index == -1 && tmp_str.compare("") == 0)
				break;
			else
				_addTopic(resultStr + "/#", cep);
		}

	}
	bool tinyServer::_addTopic(const std::string& topic, clientEventProcessor* cep)
	{


		if (_topicSubscriber.find(topic) != _topicSubscriber.end())
		{
			if (_topicSubscriber[topic] == NULL)
				_topicSubscriber[topic] = new std::set<clientEventProcessor*>;

			_topicSubscriber[topic]->insert(cep);
		}
		_topicSubscriber[topic] = new std::set<clientEventProcessor*>;
		_topicSubscriber[topic]->insert(cep);
		return true;
	}
	tinyServer* tinyServer::_instance = NULL;
}
