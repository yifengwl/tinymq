#ifndef _CLIENTEVENTPROCESSER_H_
#define _CLIENTEVENTPROCESSER_H_
namespace tinymq{

	enum phase
	{
		FIXHD_BEGIN,
		FIXHD_REMAIN_LENGTH,
		PAYLOAD
	};
	class clientEventProcessor : public eventProcessor
	{
	public:
		clientEventProcessor(tinySocket *tSock);
		~clientEventProcessor();
		 void handleReadEvent();
		 void handleWriteEvent();
		 int getSocketHandle();
		
	private:
		void closeAndClearClient();
		int fixedHeaderProcess();
		int variableHeaderProcess();
		
	private:
		void messageDispatcher();	
		void onConnect();
		void onPublish();
		void onPubAck();
		void onPubRec();
		void onPubRel();
		void onPubComp();
		void onSubscribe();
		void onUnsubscarube();
		void onPingRsp();
		void onDisconnect();
		
	private:
		void * _payload;
		char _command;
		int _readByteCount;			 //读取字节数，包括头部
		int _payloadLen;				
		int _remaining_length;				 
		int _remaining_mult;
		int _remainLenSectionCount; //mqtt头部剩余长度指示位所占字节数，协议规定<=4
		phase _nextPhase;
		
	};
}
#endif
