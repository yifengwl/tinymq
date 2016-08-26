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
		tinyServer::instance()->eraseSession(_clientId);
		delete _sock->getProcessor();
		while(!_messgaeSendQueue.empty())
		{
			free(_messgaeSendQueue.front());
			_messgaeSendQueue.pop();
		}
		for (auto it : waitingPubAckMap)
		{
			delete(it.second);
		}
		for (auto it : waitingPubRecMap)
		{
			delete(it.second);
		}
		if( _willmsg!= NULL) delete _willmsg;
			
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
		_willmsg = wm;
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
