#include "tinymq.h"

namespace tinymq {
	serverEventProcessor::serverEventProcessor(tinySocket *tSock)
	{
		this->_ownerSock = tSock;
	}
	serverEventProcessor::~serverEventProcessor()
	{
	}
	void serverEventProcessor::handleReadEvent()
	{
		socklen_t len = sizeof(sockaddr_in);
		struct sockaddr_in *sa = (struct sockaddr_in*)malloc(len) ;
		
		int clientfd = accept(this->_ownerSock->getSocketHandle(), (struct sockaddr *)sa, &len);
		if (clientfd == -1)
		{
			std::cout << "accept ERROR" << std::endl;
			return ;
		}

		tinySocket *newClient = new tinySocket(clientfd, sa, this->_ownerSock->getEpollEvent());
		clientEventProcessor * svp = new clientEventProcessor(newClient);
		newClient->setProcessor(static_cast<eventProcessor *>(svp));
		
		
		newClient->getEpollEvent()->addEvent(newClient, true, false);
		return;
		
	}
	void serverEventProcessor::handleWriteEvent()
	{
		return ;
	}

}