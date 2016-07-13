#ifndef _TINYMQ_SERVER_H_
#define _TINYMQ_SERVER_H_

#include <string>
#include <map>
namespace tinymq {

	class tinyServer
	{
	public:
		static tinyServer* instance();
		void deleteSession(const std::string& clientid);
		void addSession(const std::string& clientid, tinySession * session);
		void reuseSession(const std::string& clientid, tinySocket* sock);
	private:
		tinyServer() {};
		std::map<std::string, tinySession*> _sessions;
		static tinyServer* _instance;
	};

}


#endif /*TINYMQ_SERVER_H_*/