#ifndef _TINYMQ_H
#define _TINYMQ_H

#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>
#include <fcntl.h>  

#include <iostream>
#include <list>
#include <queue>
#include <vector>
#include <string>
#include <ext/hash_map>

namespace tinymq {

	class message;
	class epollEvent;
	class tinySocket;
	class mqServer;
	class clientIOEvent;
	class serverIOEvent;
	class eventProcessor;
	class clientEventProcessor;
	class serverEventProcessor;
}

#include "message.h"
#include "epollEvent.h"
#include "server.h"
#include "tinySocket.h"
#include "eventProcessor.h"
#include "clientEventProcessor.h"
#include "serverEventProcessor.h"
#include "mqttProtocol.h"
#define  TINY_SUCCESS   0
#define  TINY_SOCKET_ERROR   1
#define  TINY_EAGAIN   2
#define  TINY_CONNECT_LOST   3
#define  TINY_ERROR   4


#endif
