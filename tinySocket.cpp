#include "tinymq.h"
#include <stdlib.h>

namespace tinymq {	
	tinySocket::tinySocket(int sock, sockaddr_in* sockaddr, epollEvent * epoll)
	{
		this->_sockfd = sock; 
		this->_address = strdup(inet_ntoa(sockaddr->sin_addr));
		this->_epollevent = epoll;
	}
	tinySocket::~tinySocket()
	{
		_epollevent->removeEvent(this);	
		::close(_sockfd);
		free(_address);
	}

	bool tinySocket::makeSocketEnableWrite()
	{
		return _epollevent->setEvent(this, true, true);
	}

	bool tinySocket::makeSocketDisableWrite()
	{
		return _epollevent->setEvent(this, true, false);
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
	char* tinySocket::getAddress()
	{
		return this->_address;
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

