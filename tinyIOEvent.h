#ifndef _TINYMQ_IOEVENT_H_
#define _TINYMQ_IOEVENT_H_
#include <netinet/in.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <arpa/inet.h>  
#include <unistd.h>  
#include <fcntl.h>  
#include <iostream>
#include "epollEvent.h"
#include "buffer.h"
namespace tinymq{	
	class serverIOEvent : public tinyIOEvent
	{
	public:
		serverIOEvent(int sock, sockaddr_in& sockaddr, epollEvent * epoll);
		bool handleReadEvent();
		bool handleWriteEvent(){return true; }
		int getSocketHandle();
		~serverIOEvent()
		{
		}
	};
	
	class clientIOEvent : public tinyIOEvent
	{
	public:
		clientIOEvent(int sock, sockaddr_in& sockaddr, epollEvent * epoll);
		bool handleReadEvent();
		bool handleWriteEvent(){return true;}
		int getSocketHandle();
		
		~clientIOEvent()
		{
		}
	private:
		void messageDispatcher();
		void onConnect();		

	private:	
		DataBuffer dbuffer;
	};
	
	
	
	
}
#endif