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
