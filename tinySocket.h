#ifndef _TINYMQ_IOEVENT_H_
#define _TINYMQ_IOEVENT_H_
//#include "message.h"
namespace tinymq {	
		
	class tinySocket
	{
	public:
		tinySocket(int sock, sockaddr_in* sockaddr, epollEvent * epoll);
		~tinySocket();
		int getSocketHandle();
		eventProcessor *getProcessor();
		void setProcessor(eventProcessor *);
		epollEvent* getEpollEvent();
		char*  getAddress();

		bool makeSocketEnableWrite();
		bool makeSocketDisableWrite();

	private:
		bool makeSocketNoBlocking();
		
	private:
		eventProcessor *_processor;
		epollEvent * _epollevent;
		int _sockfd;
		char * _address;
	};
}
	
#endif

	
