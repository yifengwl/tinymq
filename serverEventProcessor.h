#ifndef _SERVEREVENTPROCESSER_H_
#define _SERVEREVENTPROCESSER_H_
namespace tinymq {

	class serverEventProcessor : public eventProcessor
	{
	public:
		serverEventProcessor(tinySocket* tSock);
		~serverEventProcessor();
		void handleReadEvent();
		void handleWriteEvent();
		int getSocketHandle();
	};
}
#endif
