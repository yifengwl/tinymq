#ifndef _TINYMQ_SESSION_H_
#define _TINYMQ_SESSION_H_

#include<queue>
#include<set>
#include<string>

namespace tinymq {
	struct message
	{
		short pack_id;
		int time;
		int qos;
		tinyPacket *tp;
	};
	struct willMessage
	{
		std::string _willTopic;
		char*  _willPayload;
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
		void setClientId(std::string&);
		void clearSock();
		bool getClearSession();
	public:
		std::queue<tinyPacket *> _messgaeSendQueue;
		std::set <std::string> topicSubscribed;
		
	private:				
		std::string _clientId;
		willMessage* _willmsg;
		tinySocket *_sock;
		bool _clearSession;
		
		//std::map<short, tinyPacket*> waitingPubAckMap;
		//std::map<short, tinyPacket*> waitingPubCompMap;
	};

}


#endif 