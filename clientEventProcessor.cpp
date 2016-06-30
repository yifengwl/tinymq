#include "tinymq.h"

namespace tinymq {
	clientEventProcessor::clientEventProcessor(tinySocket *tSock)
	{
		this->_ownerSock = tSock;
	}
	clientEventProcessor::~clientEventProcessor()
	{
	}
	bool clientEventProcessor::handleReadEvent()
	{
		_payload.ensureFree(1);
		int n =:: read(_ownerSock->getSocketHandle(), (void*)_payload.getFree(), 200);
		return true;
	}
	bool clientEventProcessor::handleWriteEvent()
	{
		return true;
	}

	bool clientEventProcessor::headerProcess()
	{
		return true;
	}
}