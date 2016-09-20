#include "tinymq.h"
namespace tinymq
{
	tinySession::tinySession(tinySocket *sock, bool clearSession)
	{
		_sock = sock;
		_clearSession = clearSession;
		_clientId = "";
		ackMutex = PTHREAD_MUTEX_INITIALIZER;
		recMutex = PTHREAD_MUTEX_INITIALIZER;
		compMutex = PTHREAD_MUTEX_INITIALIZER;
	}
	tinySession::~tinySession()
	{
		//std::string topicStr = std::string(topic);

		
		for (auto topicStr : topicSubscribed)
		{
			if (tinyServer::instance()->_topicSubscriber[topicStr] != NULL)
			{
				std::map<tinySession*, char>* tempMap = tinyServer::instance()->_topicSubscriber[topicStr];
				tempMap->erase(this);
			}
		}
		tinyServer::instance()->eraseSession(_clientId);
		delete _sock->getProcessor();		
	}
	void tinySession::setSock(tinySocket *sock)
	{
		_sock = sock;
	}

	void tinySession::setClientId(std::string& cid)
	{
		this->_clientId.append(cid);
	}
	void tinySession::setWillMsg(willMessage * wm){
		if (wm == NULL) return;
		_willmsg._willPayload = (char*)malloc(wm->_willPayloadLen);
		memcmp(_willmsg._willPayload, wm->_willPayload, wm->_willPayloadLen);
		_willmsg._willPayloadLen = wm->_willPayloadLen;
		_willmsg._willQos = wm->_willQos;
		_willmsg._willRetain = wm->_willRetain;
		_willmsg._willTopic = wm->_willTopic;
	}
	willMessage& tinySession::getWillMsg()
	{
		return _willmsg;
	}
	void tinySession::clearSock()
	{
		delete _sock->getProcessor();
		_sock = NULL;
	}
	bool tinySession::getClearSession()
	{
		return _clearSession;
	}
	clientEventProcessor* tinySession::getClientEP()
	{
		return dynamic_cast<clientEventProcessor*>(_sock->getProcessor());
	}
	void tinySession::ackMapLock() 
	{
		pthread_mutex_lock(&ackMutex);
	}
	void tinySession::ackMapUnlock()
	{
		pthread_mutex_unlock(&ackMutex);
	}
	void tinySession::recMapLock()
	{
		pthread_mutex_lock(&recMutex);
	}
	void tinySession::recMapUnlock()
	{
		pthread_mutex_unlock(&recMutex);
	}

	void tinySession::compMapLock()
	{
		pthread_mutex_lock(&compMutex);
	}
	void tinySession::compMapUnlock()
	{
		pthread_mutex_unlock(&compMutex);
	}
}
