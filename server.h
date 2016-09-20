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
#ifndef _TINYMQ_SERVER_H_
#define _TINYMQ_SERVER_H_

#include <string>
#include <map>
#include <set>
#include <ext/hash_map>
namespace tinymq {

	class tinyServer
	{
	private:
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
	public:
		static  tinyServer* instance();
		void deleteSession(const std::string& clientid);

		//不会delete对应的会话对象，仅从_sessions map中移除
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
		bool addTopicSubscriber(const std::string&, tinySession*, char qos);
		__gnu_cxx::hash_map<const std::string, std::map<tinySession*, char>*, str_hash, compare_str>  _topicSubscriber;
		std::map<std::string, tinySession*> getSessions();
		std::map<clientEventProcessor*, time_t> getWaittingConnections();

	public:
		pthread_mutex_t  sessionMutex;
		pthread_mutex_t  wConnectionMutex;

	private:
		bool _addTopic(const std::string&, tinySession*, char qos);

	private:
		tinyServer() {
			sessionMutex = PTHREAD_MUTEX_INITIALIZER;
			wConnectionMutex = PTHREAD_MUTEX_INITIALIZER;
		}
		std::map<std::string, tinySession*> _sessions; //会话id及其会话对象指针
		std::map<clientEventProcessor*, time_t> _waittingConnection;//建立socket连接后等待接受mqtt connect报文的map，如果超时则断开连接
	//	static tinyServer* _instance;
	};

}


#endif /*TINYMQ_SERVER_H_*/