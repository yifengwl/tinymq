#include "tinyIOEvent.h"
#include <stdlib.h>

namespace tinymq {	
	bool tinyIOEvent::makeSocketNoBlocking()
	{
		int flags, s;

		flags = fcntl(this->sockfd, F_GETFL, 0);
		if (flags == -1)
		{
			std::cout << "fcntl ERROR" << std::endl;
			return false;
		}

		flags |= O_NONBLOCK;
		s = fcntl(this->sockfd, F_SETFL, flags);
		if (s == -1)
		{
			std::cout << "fcntl ERROR" << std::endl;
			return false;
		}

		return true;
		
	}
	serverIOEvent::serverIOEvent(int sock, sockaddr_in& sockaddr, epollEvent * epoll)
	{
		this->sockfd = sock; 
		address.sin_family = sockaddr.sin_family;
		address.sin_addr.s_addr = sockaddr.sin_addr.s_addr;
		address.sin_port = sockaddr.sin_port;
		_epollevent = epoll;
	}
	
	int serverIOEvent::getSocketHandle()
	{
		return this->sockfd;
	}
	
	bool serverIOEvent::handleReadEvent()
	{
		struct sockaddr_in sa;
		socklen_t len = sizeof(sa);
		int clientfd = accept(this->sockfd, (struct sockaddr *)&sa, &len);
		if (clientfd == -1)
		{
			std::cout << "accept ERROR" << std::endl;
			return false;
		}
		

		if (!makeSocketNoBlocking())
			return false;
		
		tinyIOEvent *tempEv = new clientIOEvent(clientfd, sa, this->_epollevent);
				
		return this->_epollevent->addEvent(tempEv, true, false);
		
	}
	clientIOEvent::clientIOEvent(int sock, sockaddr_in& sockaddr, epollEvent * epoll)
	{
		this->sockfd = sock; 
		address.sin_family = sockaddr.sin_family;
		address.sin_addr.s_addr = sockaddr.sin_addr.s_addr;
		address.sin_port = sockaddr.sin_port;
		_epollevent = epoll;
	}
		
	int clientIOEvent::getSocketHandle()
	{
		return this->sockfd;
	}
	
	bool clientIOEvent::handleReadEvent()
	{
		dbuffer.ensureFree(200);
		int n = read(sockfd, (void*)dbuffer.getFree(), 200);
		 
		return true;
	}
	void clientIOEvent::messageDispatcher()
	{
	}
	
	void clientIOEvent::onConnect()
	{
	}
}