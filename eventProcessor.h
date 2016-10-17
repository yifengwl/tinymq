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
#ifndef _EVENTPROCESSER_H_
#define _EVENTPROCESSER_H_
namespace tinymq {

	class eventProcessor
	{
	public:
		virtual ~eventProcessor() {}
		virtual bool handleReadEvent() = 0;
		virtual bool handleWriteEvent() = 0;
		bool _errorOccurred;
		bool _readOccurred;
		bool _writeOccurred;
	protected:
		tinySocket * _ownerSock;
	};
}
#endif

