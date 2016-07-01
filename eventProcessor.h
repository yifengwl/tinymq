#ifndef _EVENTPROCESSER_H_
#define _EVENTPROCESSER_H_
namespace tinymq {

	class eventProcessor
	{
	public:
		virtual ~eventProcessor() {}
		virtual void handleReadEvent() = 0;
		virtual void handleWriteEvent() = 0;
		bool _errorOccurred;
		bool _readOccurred;
		bool _writeOccurred;
	protected:
		tinySocket * _ownerSock;		
	};
}
#endif
