#ifndef _CLIENTEVENTPROCESSER_H_
#define _CLIENTEVENTPROCESSER_H_
namespace tinymq{

	enum phase
	{
		START
		
	};
	class clientEventProcessor : public eventProcessor
	{
	public:
		clientEventProcessor(tinySocket *tSock);
		~clientEventProcessor();
		 bool handleReadEvent();
		 bool handleWriteEvent();
		 int getSocketHandle();
	private:
		bool headerProcess();
	//	void messageDispatcher();
	//	void onConnect();	
		
	private:
		message _payload;
		
		unsigned char _command;
		int _readByteCount;			 //读取字节数，包括头部
		int _payloadLen;				
		int _remainLen;				 //剩余未读取payload长度
		int _remainLenSectionCount; //mqtt头部剩余长度指示位所占字节数，协议规定<=4
		phase _phase;
		
	};
}
#endif