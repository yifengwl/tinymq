#include "tinymq.h"

namespace tinymq {
	clientEventProcessor::clientEventProcessor(tinySocket *tSock)
	{
		this->_ownerSock = tSock;
		_packet = NULL;
		_session = NULL;
		_errorOccurred = false;
		_readOccurred = false;
		_writeOccurred = false;

	}
	clientEventProcessor::~clientEventProcessor()
	{
		if(_packet!= NULL)delete _packet;
		if (_ownerSock != NULL)delete _ownerSock;
	}
	void clientEventProcessor::setSession(tinySession* session)
	{
		_session = session;
	}
	tinySession* clientEventProcessor::getSession()
	{
		return _session;
	}
	void clientEventProcessor::closeAndClearSession()
	{
		if (_session != NULL)
			delete _session;
		else
			delete this;
	
	}
	void clientEventProcessor::closeSockKeepSession()
	{
		_session->setSock(NULL);
		if (_ownerSock != NULL)delete _ownerSock;
	}
	bool clientEventProcessor::handleReadEvent()
	{
		if (_packet == NULL) _packet = new tinyPacket();
		
		int rc = fixedHeaderProcess();
		if (rc == TINY_EAGAIN) {
			_readOccurred = false;
			return true;
		}
			
		else if (rc == TINY_SOCKET_ERROR || rc == TINY_CONNECT_LOST)
		{
			closeAndClearSession();
			return false;
		}	
		
		rc = messageDispatcher();
		if (rc == TINY_SOCKET_ERROR || rc == TINY_ERROR)
		{
			closeAndClearSession();
			return false;
		}

		_readOccurred = false;
		return true;
	}
	bool clientEventProcessor::handleWriteEvent()
	{
		bool saveEvent = false;
		while (!_session->_messgaeSendQueue.empty())
		{
			tinyPacket *tp = _session->_messgaeSendQueue.back();
			int rc = tinyPacketWrite(tp);
			if (rc == TINY_ERROR)
			{
				closeAndClearSession();
				return false;
			}else if (rc == TINY_EAGAIN) {
				_writeOccurred = false;
				return true;
			}
			_session->_messgaeSendQueue.pop();
		}
		_ownerSock->makeSocketDisableWrite();
		_writeOccurred = false;
		return true;
	}
	int clientEventProcessor::tinyPacketWrite(tinyPacket * tp)
	{
		assert(tp);
		
		int writeLength = write(_ownerSock->getSocketHandle(),(void*)(tp->_payload + tp->_payloadPos), tp->_payloadLen - tp->_payloadPos);
		if (writeLength > 0)
		{
			if (writeLength == tp->_payloadLen - tp->_payloadPos)
				return TINY_SUCCESS;
			else {        
				tp->_payloadPos += writeLength;
				return TINY_EAGAIN;
			}
		}
		else {
			if (errno = EAGAIN)
				return TINY_EAGAIN;
			else
				return TINY_ERROR;
		}
	}

	int clientEventProcessor::messageDispatcher()
	{
		switch ((_packet->_command) & 0xF0) {
		case PINGREQ:
			return onConnect();
		case PINGRESP:
			return onPingRsp();
		case PUBACK:
			return onPubAck();
		case PUBCOMP:
			return onPubComp();
		case PUBLISH:
			return onPublish();
		case PUBREC:
			return onPubRec();
		case PUBREL:
			return onPubRel();
		case CONNECT:
			return onConnect();
		case DISCONNECT:
			return onDisconnect();
		case SUBSCRIBE:
			return onSubscribe();
		case UNSUBSCRIBE:
			return onUnsubscarube();
		case CONNACK:

		case SUBACK:

		case UNSUBACK:


		default:
			
			return 0;
		}
	}
	int clientEventProcessor::onConnect()
	{
		tinyServer::instance()->removeWaittingSock(this);
		char sessionPresent = 0x00;// connectAck中会用到
		char * protoclName = NULL;
		char * clientId = NULL;
		char * willTopic = NULL;
		char * willPayload = NULL;
		char  protocol_version;
		std::string clientidStr;
		willMessage * willmsg = NULL;
		char flag;
		short  keepAlive, willPayloadLen;
		int willQos;
		bool clearSession, willFlag, willRetain, passwordFlag, userNameFlag;

		int rc;
		if (readString(&protoclName) != TINY_SUCCESS || protoclName == NULL)
		{
			goto tiny_erro_occur;
		}
		if (readByte(&protocol_version)!= TINY_SUCCESS)
		{
			goto tiny_erro_occur;
		}

		if (!strcmp(protoclName, PROTOCOL_NAME_v31)) {
			if ((protocol_version & 0x7F) != PROTOCOL_VERSION_v31) {
				std::cout << " Invalid protocol version" << (int)protocol_version << "in CONNECT from " << this->_ownerSock->getAddress() << std::endl;
				onConnectAck(CONNACK_REFUSED_PROTOCOL_VERSION, sessionPresent);
				goto tiny_erro_occur;
			}
			_packet->_protocol = mosq_p_mqtt31;
		}
		else if (!strcmp(protoclName, PROTOCOL_NAME_v311)) {
			if ((protocol_version & 0x7F) != PROTOCOL_VERSION_v311) {
				std::cout << " Invalid protocol version" << (int)protocol_version << "in CONNECT from " << this->_ownerSock->getAddress() << std::endl;
				onConnectAck(CONNACK_REFUSED_PROTOCOL_VERSION, sessionPresent);
				goto tiny_erro_occur;
			}
			if ((_packet->_command & 0x0F) != 0x00) {
				goto tiny_erro_occur;
		
			}
			_packet->_protocol = mosq_p_mqtt311;
		}
		else
		{
			std::cout << " Invalid protocol version" << (int)protocol_version << "in CONNECT from " << this->_ownerSock->getAddress() << std::endl;
			goto tiny_erro_occur;
		}
		
		if (readByte(&flag) != TINY_SUCCESS )
		{
			goto tiny_erro_occur;
		}
		if (readShort(keepAlive) != TINY_SUCCESS)
		{
			goto tiny_erro_occur;
		}
		if (readString(&clientId) != TINY_SUCCESS || clientId == NULL ||strlen(clientId) == 0)
		{
			onConnectAck(CONNACK_REFUSED_IDENTIFIER_REJECTED, sessionPresent);
			goto tiny_erro_occur;
		}
		
		clientidStr.append(clientId);

		
		_keepAlive = keepAlive;

		if (flag & 0x01)
		{
			goto tiny_erro_occur;
		}		

		if (flag & 0x02){
			clearSession = true;
			tinyServer::instance()->deleteSession(clientidStr);
			_session = new tinySession(_ownerSock, clearSession);
			tinyServer::instance()->addSession(clientidStr, _session);
		}
		else {
			clearSession = false;
			if (tinyServer::instance()->reuseSession(clientidStr, _ownerSock))
			{
				sessionPresent = 0x01;
			}

		}

		_session->setClientId(clientidStr);


		if(flag& 0x04){
			willFlag = true;
			willQos = (flag & 0x18) >> 3;
			if (willQos == 3) goto tiny_erro_occur;

			willmsg = new willMessage();

			willmsg->_willQos = willQos;
			if (readString(&willTopic) != TINY_SUCCESS || willTopic == NULL)
			{
				goto tiny_erro_occur;
			}
			willmsg->_willTopic = std::string(willTopic);
			if (flag & 0x20) {
				willRetain = true;
			}
			else {
				willRetain = false;
			}
			willmsg->_willRetain = willRetain;
			if (readShort(willPayloadLen) != TINY_SUCCESS) 
			{
				goto tiny_erro_occur;
			}
			willPayload = (char*)malloc(willPayloadLen);
			readBytes(willPayload, willPayloadLen);
			willmsg->_willPayload = willPayload;
			_session->setWillMsg(willmsg);
		}
		else {
			_session->setWillMsg(NULL);
			willFlag = false;
			if ((flag & 0x18) || (flag & 0x20))
				goto tiny_erro_occur;
		}

		if (flag & 0x40) {
			passwordFlag = true;
		}
		else {
			passwordFlag = false;
		}
		if (flag & 0x80) {
			userNameFlag = true;
		}
		else {
			userNameFlag = false;
		}


		if (protoclName != NULL) free(protoclName);
		if (clientId != NULL) free(clientId);
		if (willTopic != NULL) free(willTopic);

		rc=onConnectAck(CONNACK_ACCEPTED, sessionPresent);
		delete _packet;
		_packet = NULL;
		return rc;

	tiny_erro_occur:
		if (protoclName != NULL) free(protoclName);
		if (clientId != NULL) free(clientId);
		if (willTopic != NULL) free(willTopic);
		if (willPayload != NULL) free(willPayload);
		if (willmsg != NULL) delete willmsg;
		return TINY_ERROR;
	}
	int clientEventProcessor::onConnectAck(char backCode, char sessionPre) {
		tinyPacket* tp = new tinyPacket();
		tp->_command = CONNACK;
		tp->_remaining_length = 2;
		tinyPacketAlloc(tp);
		tp->_payload[tp->_payloadPos] = sessionPre;
		tp->_payloadPos++;
		tp->_payload[tp->_payloadPos] = backCode;
		tp->_payloadPos = 0;
		_session->_messgaeSendQueue.push(tp);
		_ownerSock->makeSocketEnableWrite();
		return 0;
	}
	int clientEventProcessor::onPublish()
	{
		return TINY_ERROR;
	}
	int clientEventProcessor::onPubAck()
	{
		return TINY_ERROR;
	}
	int clientEventProcessor::onPubRec()
	{
		return TINY_ERROR;
	}
	int clientEventProcessor::onPubRel()
	{
		return TINY_ERROR;
	}
	int clientEventProcessor::onPubComp()
	{
		return TINY_ERROR;
	}
	int clientEventProcessor::onSubscribe()
	{
		short mid =0;
		char qos;
		char* topic;
		if(_packet->_command & 0x0F != 0x02)
			return TINY_ERROR;
		if (readShort(mid) != TINY_SUCCESS )
		{
			return TINY_ERROR;
		}
		while (_packet->_payloadPos < _packet->_remaining_length)
		{
			if (readString(&topic) != TINY_SUCCESS || topic == NULL)
			{
				return TINY_ERROR;
			}
			if (readByte(&qos) != TINY_SUCCESS|| qos>2)
			{
				free(topic);
				return TINY_ERROR;
			}
			std::string topicStr(topic);
			free(topic);
			_session->topicSubscribed.insert(topicStr);
			tinyServer::instance()->addTopicSubscriber(topicStr, this);
		}

	
	
	}
	int clientEventProcessor::onUnsubscarube()
	{
		return TINY_ERROR;
	}
	int clientEventProcessor::onPingRsp()
	{
		return TINY_ERROR;
	}
	int clientEventProcessor::onDisconnect()
	{
		return TINY_ERROR;
	}
	int clientEventProcessor::fixedHeaderProcess()
	{
		char Byte;

		switch (_packet->_nextPhase)
		{
		case FIXHD_BEGIN:
			{				
				int read_length =:: read(_ownerSock->getSocketHandle(), (void*)&Byte, 1);
				if (read_length == 1)
				{
					_packet->_remainLenSectionCount = 0;
					_packet->_remaining_length = 0;
					_packet->_remaining_mult = 1;
					_packet->_command = Byte;
					_packet->_nextPhase = FIXHD_REMAIN_LENGTH;
				}
				else
				{
					if (read_length == 0) return TINY_CONNECT_LOST;
				
					if (errno == EAGAIN) 
					{
						return TINY_EAGAIN;
					}
					else 
					{
						return TINY_SOCKET_ERROR;
					}
				}
			}
		
		case FIXHD_REMAIN_LENGTH:
			{
				do {
					int read_length =:: read(_ownerSock->getSocketHandle(), (void*)&Byte, 1);
					if (read_length == 1) {
						++_packet->_remainLenSectionCount;

						if (_packet->_remainLenSectionCount > 4) return TINY_SOCKET_ERROR;


						_packet->_remaining_length += (Byte & 127) * _packet->_remaining_mult;
						_packet->_remaining_mult *= 128;
					}
					else {
						if (read_length == 0) return TINY_CONNECT_LOST; 

						if (errno == EAGAIN) {
							return TINY_EAGAIN;
						}
						else
						{
							return TINY_SOCKET_ERROR;

						}
					}
				} while ((Byte & 128) != 0);
				
				_packet->_nextPhase = PAYLOAD;	
				if (_packet->_payload == NULL)
				{
					_packet->_payload = (char *)malloc(_packet->_remaining_length);
				}			
				else
				{
					free(_packet->_payload);
					_packet->_payload = (char *)malloc(_packet->_remaining_length);
				}
			}
		case PAYLOAD:
			{
				int read_length =:: read(_ownerSock->getSocketHandle(),
					(void*)((char *)_packet->_payload + _packet->_readByteCount),
					_packet->_remaining_length);
				if (read_length == _packet->_remaining_length)
				{
					_packet->_nextPhase = FIXHD_BEGIN;
					return TINY_SUCCESS;
				}
				else
				{
					if (read_length > 0 && read_length < _packet->_remaining_length)
					{
						_packet->_readByteCount += read_length;
						_packet->_remaining_length -= read_length;
						return TINY_EAGAIN;
					}
					else
					{
						if (read_length == 0) return TINY_SOCKET_ERROR;

						if (errno == EAGAIN) {
							return TINY_EAGAIN;
						}
						else
						{
							return TINY_SOCKET_ERROR;

						}
					}
				}
			}		
		}
		
	}
	int clientEventProcessor::readShort(short& word)
	{
		char msb, lsb;
		//_packet->_payloadPos = _packet->_payloadPos + 2;
		//if (_packet->_payloadPos + 2 > _packet->_readByteCount) return TINY_SOCKET_ERROR;
		msb = ((char*)_packet->_payload)[_packet->_payloadPos];
		_packet->_payloadPos++;
		lsb = *((char*)_packet->_payload + _packet->_payloadPos);
		_packet->_payloadPos++;
		
		word = (msb << 8) + lsb;
		
		return TINY_SUCCESS;
	}
	int clientEventProcessor::readString(char **str)
	{
		short len;
		if (readShort(len) != TINY_SUCCESS)
			return TINY_SOCKET_ERROR;
		
		*str = (char*)malloc(len+1);
		if (*str) {
			memcpy(*str, &(((char*)_packet->_payload)[_packet->_payloadPos]), len);
			(*str)[len] = '\0';
			_packet->_payloadPos += len;
			return TINY_SUCCESS;
		}
		else {
			return TINY_SOCKET_ERROR;
		}
		
	}

	int clientEventProcessor::readByte(char *byte)
	{

		memcpy(byte, &(((char*)_packet->_payload)[_packet->_payloadPos]), 1);
		_packet->_payloadPos += 1;
		return TINY_SUCCESS;
	}

	int clientEventProcessor::readBytes(char *byte, int len)
	{

		memcpy(byte, &(((char*)_packet->_payload)[_packet->_payloadPos]), len);
		_packet->_payloadPos += len;
		return TINY_SUCCESS;
	}
	int clientEventProcessor::tinyPacketAlloc(tinyPacket* tp)
	{
		char remainByte[5], byte;
		int remainLength;
		assert(tp);
		remainLength = tp->_remaining_length;
		tp->_payload = NULL;
		tp->_remainLenSectionCount = 0;
		do {
			byte = remainLength % 128;
			remainLength = remainLength / 128;
			if(remainLength >0){
				byte =	byte|0x80;
			}
			remainByte[tp->_remainLenSectionCount] = byte;
			tp->_remainLenSectionCount++;
		} while (remainLength > 0 && tp->_remainLenSectionCount < 5);

		if (tp->_remainLenSectionCount == 5)  return TINY_ERROR;
		tp->_payloadLen = tp->_remainLenSectionCount + 1 + tp->_remaining_length;

		tp->_payload = (char *)malloc(tp->_payloadLen);
		if(tp->_payload == NULL) return TINY_ERROR;

		tp->_payload[0] = tp->_command;
		for (int i = 0; i < tp->_remainLenSectionCount; i++) {
			tp->_payload[i + 1] = remainByte[i];
		}
		tp->_payloadPos = 1 + tp->_remainLenSectionCount;


		return TINY_SUCCESS;
	}

}