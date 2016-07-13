#ifndef _CLIENTEVENTPROCESSER_H_
#define _CLIENTEVENTPROCESSER_H_
namespace tinymq{
	enum phase
	{
		FIXHD_BEGIN,
		FIXHD_REMAIN_LENGTH,
		PAYLOAD
	};
	enum tiny_protocol {
		mosq_p_invalid = 0,
		mosq_p_mqtt31 = 1,
		mosq_p_mqtt311 = 2,
		mosq_p_mqtts = 3
	};
	struct tinyPacket
	{
		void * _payload;
		int _payloadLen;
		
		char _command;
		int _readByteCount;			 //payload读取字节数
		int _remaining_length;				 
		int _remaining_mult;
		int _remainLenSectionCount;	//mqtt头部剩余长度指示位所占字节数，协议规定<=4
		tiny_protocol _protocol;
		int _payloadPos;// 解析_payload所在位置
		phase _nextPhase;
		
		tinyPacket()
		{
			_nextPhase = FIXHD_BEGIN;
			_payload = NULL;
			_payloadLen = 0;				
			_remaining_length = 0;	
			_remaining_mult = 1;
			_remainLenSectionCount = 0;
			_payloadPos = 0;
		}
		~tinyPacket()
		{
			free(_payload);
		}
	};

	class clientEventProcessor : public eventProcessor
	{
	public:
		clientEventProcessor(tinySocket *tSock);
		~clientEventProcessor();
		 void handleReadEvent();
		 void handleWriteEvent();
		 int getSocketHandle();
		 void setSession(tinySession* session);
		 int _keepAlive;

		 void closeSockKeepSession();
	private:
		void closeAndClearSession();
	
		int fixedHeaderProcess();
		int variableHeaderProcess();
		
		
	private:
		int messageDispatcher();	
		int onConnect();
		int onPublish();
		int onPubAck();
		int onPubRec();
		int onPubRel();
		int onPubComp();
		int onSubscribe();
		int onUnsubscarube();
		int onPingRsp();
		int onDisconnect();
		
	private:	
		inline int readShort(short& word);
		inline int readString(char **str);
		inline int readByte(char *byte);
			
		
	private:
		
		struct tinyPacket * _packet;
		tinySession* _session;
		
		//void * _payload;
		//char _command;
		//int _readByteCount;			 //payload读取字节数
		//int _payloadLen;				
		//int _remaining_length;				 
		//int _remaining_mult;
		//int _remainLenSectionCount;	//mqtt头部剩余长度指示位所占字节数，协议规定<=4
		//
		//int _payloadPos;// 解析_payload所在位置
		//phase _nextPhase;
		
	};
}
#endif
