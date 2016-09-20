/*
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
*
* Version:1.0
*
* Authors:
*   wanglu <1048274411@qq.com>
*
*/
#ifndef _TINYMQ_IOEVENT_H_
#define _TINYMQ_IOEVENT_H_
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
		sockaddr_in*  getAddress();


		bool makeSocketEnableWrite();
		bool makeSocketDisableWrite();

	private:
		bool makeSocketNoBlocking();
		
	private:
		eventProcessor *_processor;
		epollEvent * _epollevent;
		int _sockfd;
		struct sockaddr_in * _address;

	};
}
	
#endif

	
