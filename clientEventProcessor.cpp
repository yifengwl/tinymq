#include "tinymq.h"

namespace tinymq {
	clientEventProcessor::clientEventProcessor(tinySocket *tSock)
	{
		this->_ownerSock = tSock;
		_packet = NULL;
		_session = NULL;
		//_nextPhase = FIXHD_BEGIN;
		//_payload = NULL;
		//_payloadLen = 0;				
		//_remaining_length = 0;	
		//_remaining_mult = 1;
		//_remainLenSectionCount = 0;
	}
	clientEventProcessor::~clientEventProcessor()
	{
		delete _packet;
		delete _ownerSock;
	}
	void clientEventProcessor::setSession(tinySession* session)
	{
		_session = session;
	}
	void clientEventProcessor::closeAndClearSession()
	{
		if(_session != NULL)
			delete _session;
		delete this;	
	}
	void clientEventProcessor::closeSockKeepSession()
	{
		_session->setSock(NULL);
		delete this;
	}
	void clientEventProcessor::handleReadEvent()
	{
		if (_packet == NULL) _packet = new tinyPacket();
		
		int rc = fixedHeaderProcess();
		if (rc == TINY_EAGAIN)
			return;
		else if (rc == TINY_SOCKET_ERROR || rc == TINY_CONNECT_LOST)
		{
			closeAndClearSession();
			return;
		}	
		
		messageDispatcher();
	}
	void clientEventProcessor::handleWriteEvent()
	{
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
		char * protoclName = NULL;
		char * clientId = NULL;
		char  protocol_version;
		char flag;
		short  keepAlive;
		int willQos;
		bool clearSession, willFlag, willRetain, passwordFlag, userNameFlag;

		int rc;
		if (readString(&protoclName) != TINY_SUCCESS || protoclName == NULL)
		{
			return TINY_ERROR;
		}
		if (readByte(&protocol_version)!= TINY_SUCCESS)
		{
			return TINY_ERROR;
		}

		if (!strcmp(protoclName, PROTOCOL_NAME_v31)) {
			if ((protocol_version & 0x7F) != PROTOCOL_VERSION_v31) {
				std::cout << " Invalid protocol version" << (int)protocol_version << "in CONNECT from " << this->_ownerSock->getAddress() << std::endl;
				rc = TINY_ERROR;
			}
			_packet->_protocol = mosq_p_mqtt31;
		}
		else if (!strcmp(protoclName, PROTOCOL_NAME_v311)) {
			if ((protocol_version & 0x7F) != PROTOCOL_VERSION_v311) {
				std::cout << " Invalid protocol version" << (int)protocol_version << "in CONNECT from " << this->_ownerSock->getAddress() << std::endl;
				rc = TINY_ERROR;
			}
			if ((_packet->_command & 0x0F) != 0x00) {
				rc = TINY_ERROR;
		
			}
			_packet->_protocol = mosq_p_mqtt311;
		}
		else
		{
			std::cout << " Invalid protocol version" << (int)protocol_version << "in CONNECT from " << this->_ownerSock->getAddress() << std::endl;
			rc = TINY_ERROR; 
		}
		
		if (readByte(&flag) != TINY_SUCCESS )
		{
			return TINY_ERROR;
		}
		if (readShort(keepAlive) != TINY_SUCCESS)
		{
			return TINY_ERROR;
		}
		if (readString(&clientId) != TINY_SUCCESS || protoclName == NULL)
		{
			return TINY_ERROR;
		}
		std::string clientidStr(clientId);

		_keepAlive = keepAlive;

		if (flag & 0x01)
		{

			return TINY_ERROR;
		}



		if (flag & 0x02){
			clearSession = true;
			tinyServer::instance()->deleteSession(clientidStr);
			_session = new tinySession(_ownerSock, clearSession);
			tinyServer::instance()->addSession(clientidStr, _session);
		}
		else {
			clearSession = false;

		}
		if(flag& 0x04){
			willFlag = true;
			willQos = (flag & 0x18) >> 3;
			if (flag & 0x20) {
				willRetain = true;
			}
			else {
				willRetain = false;
			}
		}
		else {
			willFlag = false;
			if ((flag & 0x18) || (flag & 0x20))
				return TINY_ERROR;
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
		return TINY_ERROR;
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
					_packet->_payload = malloc(_packet->_remaining_length);
				}			
				else
				{
					free(_packet->_payload);
					_packet->_payload = malloc(_packet->_remaining_length);
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
						if (read_length == 0) return TINY_CONNECT_LOST; 

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
}