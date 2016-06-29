#include "epollEvent.h"
#include <string.h>
namespace tinymq {
	epollEvent::epollEvent()
	{
		_iepfd = epoll_create(MAX_SOCKET_EVENTS);
	}
	epollEvent::~epollEvent()
	{
		close(_iepfd);
	}
	bool epollEvent::addEvent(tinyIOEvent *socket, bool enableRead, bool enableWrite)
	{
		struct epoll_event ev;
		memset(&ev, 0, sizeof(ev));
		ev.data.ptr = socket;
		ev.events = 0;
		if (enableRead) {
			ev.events |= EPOLLIN;
		}
		if (enableWrite) {
			ev.events |= EPOLLOUT;
		}

		bool rc = (epoll_ctl(_iepfd, EPOLL_CTL_ADD, socket->getSocketHandle(), &ev) == 0);	
		return rc;
	}
	
	bool epollEvent::setEvent(tinyIOEvent *socket, bool enableRead, bool enableWrite) 
	{

		struct epoll_event ev;
		memset(&ev, 0, sizeof(ev));
		ev.data.ptr = socket;
		ev.events = 0;

		if (enableRead) {
			ev.events |= EPOLLIN;
		}
		if (enableWrite) {
			ev.events |= EPOLLOUT;
		}
		bool rc = (epoll_ctl(_iepfd, EPOLL_CTL_MOD, socket->getSocketHandle(), &ev) == 0);
		return rc;
	}
	
	
	
	bool epollEvent::removeEvent(tinyIOEvent *socket) 
	{

		struct epoll_event ev;
		memset(&ev, 0, sizeof(ev));
		ev.data.ptr = socket;
		ev.events = 0;
		bool rc = (epoll_ctl(_iepfd, EPOLL_CTL_DEL, socket->getSocketHandle(), &ev) == 0);
		return rc;
	}
	
	int epollEvent::getEvents(int timeout, std::vector<tinyIOEvent *>& ioevents, int cnt) {

		struct epoll_event events[MAX_SOCKET_EVENTS];

		if (cnt > MAX_SOCKET_EVENTS) {
			cnt = MAX_SOCKET_EVENTS;
		}

		int res = epoll_wait(_iepfd, events, cnt, timeout);

		ioevents.clear();
	  // 把events的事件转化成tinyIOEvent的事件
		for (int i = 0; i < res; i++) {
			ioevents.push_back((tinyIOEvent*)events[i].data.ptr);
			if (events[i].events & (EPOLLERR | EPOLLHUP)) {
				ioevents[i]->_errorOccurred = true;
			}
			if ((events[i].events & EPOLLIN) != 0) {
				ioevents[i]->_readOccurred = true;
			}
			if ((events[i].events & EPOLLOUT) != 0) {
				ioevents[i]->_writeOccurred = true;
			}
		}

		return res;
	}
}