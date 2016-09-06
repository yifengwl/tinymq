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
#ifndef _SERVEREVENTPROCESSER_H_
#define _SERVEREVENTPROCESSER_H_
namespace tinymq {

	class serverEventProcessor : public eventProcessor
	{
	public:
		serverEventProcessor(tinySocket* tSock);
		~serverEventProcessor();
		bool handleReadEvent();
		bool handleWriteEvent();
		int getSocketHandle();
	};
}
#endif
