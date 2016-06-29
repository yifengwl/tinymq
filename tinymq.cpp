#include <stdio.h>  
#include <unistd.h>
#include <string.h>  
#include <iostream>  
#include <sys/socket.h>    
#include <sys/epoll.h>    
#include <netinet/in.h>      
#include <arpa/inet.h>      
#include <netdb.h>  
#include "tinyIOEvent.h"
//#include <event.h>  
#include "epollEvent.h"
using namespace std;  
using namespace tinymq;


int main()  
{  
      
	int iSvrFd, epoll_fd; 
	struct sockaddr_in sSvrAddr;  
	struct epoll_event ev;
	std::vector<tinyIOEvent *> events;
	
	memset(&sSvrAddr, 0, sizeof(sSvrAddr));    
	sSvrAddr.sin_family = AF_INET;    
	sSvrAddr.sin_addr.s_addr = inet_addr("127.0.0.1");      
	sSvrAddr.sin_port = htons(8888);     
                                  
	// 创建tcpSocket（iSvrFd），监听本机8888端口    
	iSvrFd = socket(AF_INET, SOCK_STREAM, 0);    
	bind(iSvrFd, (struct sockaddr*)&sSvrAddr, sizeof(sSvrAddr));    
	listen(iSvrFd, 10);  
	epollEvent  *epollevent = new epollEvent();
	
	tinyIOEvent *svrIOev = new serverIOEvent(iSvrFd, sSvrAddr,epollevent);
	epollevent->addEvent(svrIOev, true, false);
	//epollevent->getEvents(-1, events, MAX_SOCKET_EVENTS);
	
	for (;;)
	{
		epollevent->getEvents(-1, events, MAX_SOCKET_EVENTS);
		for (auto vc : events)
		{
			if (vc->_readOccurred)
			{
				vc->handleReadEvent();
			}
			else if (vc->_writeOccurred) 
			{	
				vc->handleWriteEvent();
			}
			else 
			{
				std::cout << "ERROR OCCURRED" << std ::endl;
			}
		}
	}
      
	return 0;  
}  