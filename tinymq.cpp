#include "tinymq.h"
#include "signal.h"

using namespace std;  
using namespace tinymq;

void * timeCheckerLoop(void* t)
{
	while (true)
	{
 		time_t now = time(NULL);
		std::map<clientEventProcessor*, time_t>::iterator wcit;
		pthread_mutex_lock(&tinyServer::instance()->wConnectionMutex);
		for (wcit =tinyServer::instance()->getWaittingConnections().begin(); wcit!= tinyServer::instance()->getWaittingConnections().end();)
		{
			if (wcit->second + TINY_WAITTING_CONNECTION_MAX_SECEND < now)
			{
				wcit->first->closeSockKeepSession();
				tinyServer::instance()->getWaittingConnections().erase(wcit++);
			}
			else
			{
				++wcit;
			}
		}
		pthread_mutex_unlock(&tinyServer::instance()->wConnectionMutex);

		std::map<std::string, tinySession*>::iterator  sit;
		pthread_mutex_lock(&tinyServer::instance()->sessionMutex);

		for (sit = tinyServer::instance()->getSessions().begin(); sit != tinyServer::instance()->getSessions().end(); sit++)
		{			
			if (sit->second->getClientEP()->isOvertime())
			{
				delete sit->second;
				tinyServer::instance()->getSessions().erase(sit);
				continue;
			}

			std::map<short, std::shared_ptr<tinymq::tinyPacket>>::iterator ackit;

			sit->second->ackMapLock();
			for (ackit = sit->second->waitingPubAckMap.begin(); ackit != sit->second->waitingPubAckMap.end(); ++ackit)
			{
				if (ackit->second->sendTime + TINY_WAITTING_PUBACK_MAX_SECEND < now)
				{
					sit->second->getClientEP()->resendPublish(ackit->second);
				}
				
			}
			sit->second->ackMapUnlock();

			sit->second->recMapLock();
			for (ackit = sit->second->waitingPubRecMap.begin(); ackit != sit->second->waitingPubRecMap.end(); ++ackit)
			{
				if (ackit->second->sendTime + TINY_WAITTING_PUBREC_MAX_SECEND < now)
				{
					sit->second->getClientEP()->resendPublish(ackit->second);
				}
		
			}
			sit->second->recMapUnlock();


			sit->second->compMapLock();
			for (ackit = sit->second->waitingPubCompMap.begin(); ackit != sit->second->waitingPubCompMap.end(); ++ackit)
			{
				if (ackit->second->sendTime + TINY_WAITTING_PUBCOMP_MAX_SECEND < now)
				{
					sit->second->getClientEP()->resendPubRel(ackit->second);
				}
			}

			sit->second->compMapUnlock();

		}
		pthread_mutex_unlock(&tinyServer::instance()->sessionMutex);

		usleep(2000000); //2s
	}
}

int main()  

{
	signal(SIGPIPE, SIG_IGN);

	sigset_t signal_mask;
	sigemptyset(&signal_mask);
	sigaddset(&signal_mask, SIGPIPE);
	int rc = pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);
	if (rc != 0)
	{
		exit(0);//printf("block sigpipe error\n");
	}
	pthread_t pid;
	int ret = pthread_create(&pid, NULL, timeCheckerLoop,NULL);
	pthread_detach(pid);

	int iSvrFd, epoll_fd; 
	struct sockaddr_in sSvrAddr;  
	struct epoll_event ev;
	std::vector<eventProcessor *> events;
	
	memset(&sSvrAddr, 0, sizeof(sSvrAddr));    
	sSvrAddr.sin_family = AF_INET;    
	sSvrAddr.sin_addr.s_addr = inet_addr("192.168.1.162");      
	sSvrAddr.sin_port = htons(8888);     

	// 创建tcpSocket（iSvrFd），监听本机8888端口    
	iSvrFd = socket(AF_INET, SOCK_STREAM, 0);    
	bind(iSvrFd, (struct sockaddr*)&sSvrAddr, sizeof(sSvrAddr));    
	listen(iSvrFd, 10);  
	epollEvent  *epollevent = new epollEvent();
	
	tinySocket *tSock = new tinySocket(iSvrFd, &sSvrAddr, epollevent);
	serverEventProcessor * svp = new serverEventProcessor(tSock);
	tSock->setProcessor(static_cast<eventProcessor *>(svp));
	epollevent->addEvent(tSock, true, false);

	for (;;)
	{
		events.clear();
		epollevent->getEvents(-1, events, MAX_SOCKET_EVENTS);
		for (auto vc : events)
		{
			if (vc->_writeOccurred)
				vc->handleWriteEvent();
			if (vc->_readOccurred)
				vc->handleReadEvent();
			if (vc->_errorOccurred)
			{
				std::cout << "ERROR OCCURRED" << std::endl;
				vc->_errorOccurred = false;
			}
				
			
		}
	}
      
	return 0;  
}  