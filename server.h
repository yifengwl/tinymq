#ifndef _TINYMQ_SERVER_H_
#define _TINYMQ_SERVER_H_

#include <string>
#include <map>
#include "tinyIOEvent.h"
namespace tinymq {

	class mqServer
	{
	public:
		void messageDispatcher();
		
	private:
		void onConnect();
		void onPublish();
		void onPubAck();
		void onPubRec();
		void onPubRel();
		void onPubComp();
		void onSubscribe();
		void onUnsubscarube();
		void onPingReq();
		void onDisconnect();
		
	private:
		std::map<std::string, clientIOEvent*> clientMap;
	};

}


#endif /*TINYMQ_SERVER_H_*/