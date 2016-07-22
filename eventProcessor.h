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
