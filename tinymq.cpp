#include "tinymq.h"
using namespace std;  
using namespace tinymq;


int main()  
{  
      
	int iSvrFd, epoll_fd; 
	struct sockaddr_in sSvrAddr;  
	struct epoll_event ev;
	std::vector<eventProcessor *> events;
	
	memset(&sSvrAddr, 0, sizeof(sSvrAddr));    
	sSvrAddr.sin_family = AF_INET;    
	sSvrAddr.sin_addr.s_addr = inet_addr("127.0.0.1");      
	sSvrAddr.sin_port = htons(8888);     
                                  
	// 创建tcpSocket（iSvrFd），监听本机8888端口    
	iSvrFd = socket(AF_INET, SOCK_STREAM, 0);    
	bind(iSvrFd, (struct sockaddr*)&sSvrAddr, sizeof(sSvrAddr));    
	listen(iSvrFd, 10);  
	epollEvent  *epollevent = new epollEvent();
	
	tinySocket *tSock = new tinySocket(iSvrFd, &sSvrAddr, epollevent);
	serverEventProcessor * svp = new serverEventProcessor(tSock);
	tSock->setProcessor(static_cast<eventProcessor *>(svp));
	epollevent->addEvent(tSock, true, false);

	for (;;)
	{
		epollevent->getEvents(-1, events, MAX_SOCKET_EVENTS);
		for (auto vc : events)
		{
			if (vc->_readOccurred)
				vc->handleReadEvent();		
			if (vc->_writeOccurred) 
				vc->handleWriteEvent();			
			if (vc->_errorOccurred)
				std::cout << "ERROR OCCURRED" << std ::endl;
			
		}
	}
      
	return 0;  
}  