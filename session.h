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
#ifndef _TINYMQ_SESSION_H_
#define _TINYMQ_SESSION_H_

#include<queue>
#include<map>
#include<set>
#include<string>


namespace tinymq {
	//struct message
	//{
	//	short pack_id;
	//	int time;
	//	int qos;
	//	tinyPacket *tp;
	//};
	struct willMessage
	{
		std::string _willTopic;
		char*  _willPayload;
		long _willPayloadLen;
		int _willQos;
		bool _willRetain;
	};

	class tinySession
	{
	public:
		tinySession(tinySocket *sock, bool clearSession);
		~tinySession();
		void setSock(tinySocket *sock);
		void setWillMsg(willMessage *);
		willMessage * getWillMsg();
		void setClientId(std::string&);
		void clearSock();
		bool getClearSession();
		clientEventProcessor* getClientEP();
	public:

		void ackMapLock();
		void ackMapUnlock();
		void recMapLock();
		void recMapUnlock();
		void compMapLock();
		void compMapUnlock();
	public:
		std::queue<tinyPacket *> _messgaeSendQueue; //待发送的消息队列
		std::set <std::string> topicSubscribed; //此会话对象订阅的话题
		std::map<short, tinyPacket*> waitingPubAckMap; //qos1级别消息等待回应
		std::map<short, tinyPacket*> waitingPubRecMap; //qos2级别消息等待接受
		std::map<short, tinyPacket*> waitingPubCompMap; //qos2级别消息等待完成
		std::set<short> midsInuse;  //在使用的mid，不可重复

	private:
		std::string _clientId;
		willMessage* _willmsg;
		tinySocket *_sock;
		bool _clearSession;
		pthread_mutex_t  ackMutex;
		pthread_mutex_t  recMutex;
		pthread_mutex_t  compMutex;

	};

}


#endif 