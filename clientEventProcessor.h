/*
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
*
* Version:1.0
*
* Authors:
*   wanglu <1048274411@qq.com>
*
*/
#ifndef _CLIENTEVENTPROCESSER_H_
#define _CLIENTEVENTPROCESSER_H_
#include<string.h>
namespace tinymq {
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
		char * _payload;
		int _payloadLen;

		char _command;
		int _readByteCount;			 //payload读取字节数
		int _remaining_length;
		int _remaining_mult;
		int _remainLenSectionCount;		//mqtt头部剩余长度指示位所占字节数，协议规定<=4
		tiny_protocol _protocol;
		int _payloadPos;		// 解析_payload所在位置
		phase _nextPhase;
		short mid;
		time_t sendTime;
		tinyPacket(const tinyPacket& tp)
		{
			_payload = (char*)malloc(tp._payloadLen);
			memcpy(_payload, tp._payload, tp._payloadLen);
			_payloadLen = tp._payloadLen;
			_command = tp._command;		
			_readByteCount = tp._readByteCount;
			_remaining_length = tp._remaining_length;
			_remaining_mult = tp._remaining_mult;
			_remainLenSectionCount = tp._remainLenSectionCount;
			_protocol = tp._protocol;
			_payloadPos = tp._payloadPos;
			_nextPhase = tp._nextPhase;
		}
		tinyPacket()
		{
			_nextPhase = FIXHD_BEGIN;
			_payload = NULL;
			_readByteCount = 0;
			_payloadLen = 0;
			_remaining_length = 0;
			_remaining_mult = 1;
			_remainLenSectionCount = 0;
			_payloadPos = 0;
		}
		~tinyPacket()
		{
			if (_payload != NULL) free(_payload);
		}
	};

	class clientEventProcessor : public eventProcessor
	{
	public:
		clientEventProcessor(tinySocket *tSock);
		~clientEventProcessor();
		bool handleReadEvent();
		bool handleWriteEvent();
		int getSocketHandle();
		void setSession(tinySession* session);
		tinySession*  getSession();
		int _keepAlive;
		void closeSockKeepSession();
		bool isOvertime();
		void updateVisitTime();
		int resendPublish(tinyPacket *);
		int resendPubRel(tinyPacket *);

	private:

		void closeAndClearSession();
		int fixedHeaderProcess();



	private:
		int tinyPacketWrite(tinyPacket *);
		int messageDispatcher();
		int onConnect();
		int onConnectAck(char, char);
		int onPublish();
		int onPubAck();
		int onPubRec();
		int onPubRel();
		int onPubComp();
		int onSubscribe();
		int onUnsubscarube();
		int onPingRsp();
		int onDisconnect();

		
		int sendSubscribeAck(std::vector<char>&, short);
		int sendPublishAck(short mid);
		int sendPublishRec(short mid);
		int sendPubComp(short);
		int sendPubRel(short);
		int sendUnsubscarubeAck(short);
	private:
		int publishToTopic(std::string&, char*, int, short);
		int acceptPublish(tinyPacket *);
	private:
		inline int readShort(short& word);
		inline int readString(char **str);
		inline int readByte(char *byte);
		inline int readBytes(char *byte, int len);
		inline int tinyPacketAlloc(tinyPacket*);
		inline int sendPublishPayload(std::string& topic, char* payload, int payloadLen, short mid, char qos, tinySession*);

	private:
		
		struct tinyPacket * _packet;
		tinySession* _session;
		time_t _lastVisitTime;
	};
}
#endif
