#include "tinymq.h"
namespace tinymq
{
	tinySession::tinySession(tinySocket *sock, bool clearSession)
	{
		_sock = sock;
		_clearSession = clearSession;
	}
	tinySession::~tinySession()
	{
		delete _sock->getProcessor();
	}
	void tinySession::setSock(tinySocket *sock)
	{
		_sock = sock;
	}
	void tinySession::clearSock()
	{
		dynamic_cast<clientEventProcessor*>(_sock->getProcessor())->closeSockKeepSession();
	}
	bool tinySession::getClearSession()
	{
		return _clearSession;
	}
}
