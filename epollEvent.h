/*
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
*
* Version: 1.0
*
* Authors:
*   wanglu <1048274411@qq.com>
*
*/
#ifndef _TINYMQ_EPOLLEVENT_H_
#define _TINYMQ_EPOLLEVENT_H_


#define MAX_SOCKET_EVENTS 200
#include <vector>
namespace tinymq{

	class epollEvent
	{
	public:
		epollEvent();
		~epollEvent();
	   /*
        * 增加tinySocket到事件中
        *
        * @param tinySocket 被加的socket
        * @param enableRead: 设置是否可读
        * @param enableWrite: 设置是否可写
        * @return  操作是否成功, true – 成功, false – 失败
        */
		bool addEvent(tinySocket *socket, bool enableRead, bool enableWrite);

	    /*
	     * 设置tinySocket到事件中
		 *
		 * @param socket 被加的socket
		 * @param enableRead: 设置是否可读
		 * @param enableWrite: 设置是否可写
		 * @return  操作是否成功, true – 成功, false – 失败
		 */
		bool setEvent(tinySocket *socket, bool enableRead, bool enableWrite);

	   /*
	    * 删除tinySocket到事件中
		*
		* @param socket 被删除socket
		* @return  操作是否成功, true – 成功, false – 失败
		*/
		bool removeEvent(tinySocket *socket);

		/*
		 * 得到读写事件。
		 *
		 * @param timeout  超时时间(单位:ms)
		 * @param events  事件数组
		 * @param cnt   events的数组大小
		 * @return 事件数, 0为超时
		 */
		int getEvents(int timeout, std::vector<eventProcessor *>& events, int cnt);
		
	private:
		int _iepfd; // epoll fd
	};
	
}

#endif
