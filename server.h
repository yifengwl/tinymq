#ifndef _TINYMQ_SERVER_H_
#define _TINYMQ_SERVER_H_

#include <string>
#include <map>
#include <set>
#include <ext/hash_map>
namespace tinymq {

	class tinyServer
	{
	public:
		static  tinyServer* instance();
		void deleteSession(const std::string& clientid);

		//不会delete对应的会话对象
		void eraseSession(const std::string& clientid);
		void addSession(const std::string& clientid, tinySession * session);
		/*
			重用会话
			返回true：找到旧会话成功重用
			返回false：没有旧会话，新建会话成功
		*/
		bool reuseSession(const std::string& clientid, tinySocket* sock);
		void addWaittingSock(clientEventProcessor *, time_t);
		void removeWaittingSock(clientEventProcessor *);
		bool addTopicSubscriber(const std::string&, clientEventProcessor*);
		//__gnu_cxx::hash_map<int, std::set<clientEventProcessor*>>  test;
	private:
		bool _addTopic(const std::string&, clientEventProcessor*);
	private:
		tinyServer() {};
		struct str_hash {
			size_t operator()(const std::string& str) const
			{
				return __gnu_cxx::__stl_hash_string(str.c_str());
			}
		};
		struct compare_str {
			bool operator()(const std::string& p1, const std::string& p2) const {
				return p1.compare(p2) == 0;
			}
		};
		__gnu_cxx::hash_map<const std::string, std::set<clientEventProcessor*>*, str_hash, compare_str>  _topicSubscriber;

		//std::map< std::string, std::set<clientEventProcessor*>>  _topicSubscriber;
		std::map<std::string, tinySession*> _sessions;
		std::map<clientEventProcessor*, time_t> _waittingConnection;
		static tinyServer* _instance;
	};

}


#endif /*TINYMQ_SERVER_H_*/