#include "tinymq.h"
namespace tinymq
{
	tinySession::tinySession(tinySocket *sock, bool clearSession)
	{
		_sock = sock;
		_clearSession = clearSession;
		_clientId = "";
	}
	tinySession::~tinySession()
	{
		tinyServer::instance()->eraseSession(_clientId);
		delete _sock->getProcessor();
		while(!_messgaeSendQueue.empty())
		{
			free(_messgaeSendQueue.back());
			_messgaeSendQueue.pop();
		}
		delete _willmsg;
			
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
}
