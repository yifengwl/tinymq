#ifndef _TINYMQ_IOEVENT_H_
#define _TINYMQ_IOEVENT_H_
//#include "message.h"
namespace tinymq {	
		
	class tinySocket
	{
	public:
		tinySocket(int sock, sockaddr_in* sockaddr, epollEvent * epoll);
		int getSocketHandle();
		eventProcessor *getProcessor();
		void setProcessor(eventProcessor *);
		epollEvent* getEpollEvent();
	private:
		bool makeSocketNoBlocking();
		
	private:
		eventProcessor *_processor;
		epollEvent * _epollevent;
		int _sockfd;
		sockaddr_in * _address;
	};
}
	
#endif

	
