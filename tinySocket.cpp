#include "tinymq.h"
#include <stdlib.h>

namespace tinymq {	
	tinySocket::tinySocket(int sock, sockaddr_in* sockaddr, epollEvent * epoll)
	{
		this->_sockfd = sock; 
		this->_address = sockaddr;
		this->_epollevent = epoll;
	}
	bool tinySocket::makeSocketNoBlocking()
	{
		int flags, s;

		flags = fcntl(this->_sockfd, F_GETFL, 0);
		if (flags == -1)
		{
			std::cout << "fcntl ERROR" << std::endl;
			return false;
		}

		flags |= O_NONBLOCK;
		s = fcntl(this->_sockfd, F_SETFL, flags);
		if (s == -1)
		{
			std::cout << "fcntl ERROR" << std::endl;
			return false;
		}
		return true;
		
	}
	int tinySocket::getSocketHandle()
	{
		return this->_sockfd;
	}
	eventProcessor *tinySocket::getProcessor()
	{
		return this->_processor;
	}
	void tinySocket::setProcessor(eventProcessor * ep)
	{
		this->_processor = ep;
	}
	epollEvent* tinySocket::getEpollEvent()
	{
		return this->_epollevent;
	}
}

	//
	//int serverIOEvent::getSocketHandle()
	//{
		//return this->_sockfd;
	//}
	//
	//bool serverIOEvent::handleReadEvent()
	//{
		//struct sockaddr_in sa;
		//socklen_t len = sizeof(sa);
		//int clientfd = accept(this->_sockfd, (struct sockaddr *)&sa, &len);
		//if (clientfd == -1)
		//{
			//std::cout << "accept ERROR" << std::endl;
			//return false;
		//}
		//
//
		//if (!makeSocketNoBlocking())
			//return false;
		//
		//tinySocket *tempEv = new clientIOEvent(clientfd, sa, this->_epollevent);
				//
		//return this->_epollevent->addEvent(tempEv, true, false);
		//
	//}
	//clientIOEvent::clientIOEvent(int sock, sockaddr_in& sockaddr, epollEvent * epoll)
	//{
		//this->_sockfd = sock; 
		//address.sin_family = sockaddr.sin_family;
		//address.sin_addr.s_addr = sockaddr.sin_addr.s_addr;
		//address.sin_port = sockaddr.sin_port;
		//_epollevent = epoll;
	//}
		//
	//int clientIOEvent::getSocketHandle()
	//{
		//return this->_sockfd;
	//}
	//
	//bool clientIOEvent::handleReadEvent()
	//{
		//headerProcess();
		//return true;
	//}
	//void clientIOEvent::headerProcess()
	//{
		//_message.ensureFree(1);
		//int n = read(_sockfd, (void*)_message.getFree(), 200);
	//}
	//
	//void clientIOEvent::messageDispatcher()
	//{
		//_message.ensureFree(1);
		//int n = read(_sockfd, (void*)_message.getFree(), 200);
	//}
	//
	//void clientIOEvent::onConnect()
	//{
	//}
