#ifndef _TINYMQ_SESSION_H_
#define _TINYMQ_SESSION_H_

#include<queue>
#include<map>
#include<string>
namespace tinymq {
	struct message
	{
		short pack_id;
		int time;
		int qos;
		tinyPacket *tp;
	};

	class tinySession
	{
	public:
		tinySession(tinySocket *sock, bool clearSession);
		~tinySession();
		void setSock(tinySocket *sock);
		void clearSock();
		bool getClearSession();
	private:		
		std::string _clientId;
		tinySocket *_sock;
		bool _clearSession;
		//std::queue<message *> _messgaeSendQueue;
		//std::map<short, message*> waitingPubAckMap;
		//std::map<short, message*> waitingPubCompMap;
	};

}


#endif 