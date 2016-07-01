#include "tinymq.h"

namespace tinymq {
	clientEventProcessor::clientEventProcessor(tinySocket *tSock)
	{
		this->_ownerSock = tSock;
		_nextPhase = FIXHD_BEGIN;
		_payload = NULL;
		_payloadLen = 0;				
		_remaining_length = 0;	
		_remaining_mult = 1;
		_remainLenSectionCount = 0;
	}
	clientEventProcessor::~clientEventProcessor()
	{
		free(_payload);
		delete _ownerSock;
	}
	void clientEventProcessor::closeAndClearClient()
	{
		delete this;	
	}
	void clientEventProcessor::handleReadEvent()
	{
		int rc = fixedHeaderProcess();
		if (rc == TINY_EAGAIN)
			return;
		else if (rc == TINY_SOCKET_ERROR || rc == TINY_CONNECT_LOST)
		{
			closeAndClearClient();
			return;
		}	
		
		messageDispatcher();
	}
	void clientEventProcessor::handleWriteEvent()
	{
	}
	void clientEventProcessor::messageDispatcher()
	{
		switch ((_command) & 0xF0) {
		case PINGREQ:
			onConnect();
		case PINGRESP:
			onPingRsp();
		case PUBACK:
			onPubAck();
		case PUBCOMP:
			onPubComp();
		case PUBLISH:
			onPublish();
		case PUBREC:
			onPubRec();
		case PUBREL:
			onPubRel();
		case CONNECT:
			onConnect();
		case DISCONNECT:
			onDisconnect();
		case SUBSCRIBE:
			onSubscribe();
		case UNSUBSCRIBE:
			onUnsubscarube();
		case CONNACK:

		case SUBACK:

		case UNSUBACK:


		default:
			
			return ;
		}
	}
	void clientEventProcessor::onConnect()
	{
	}
	void clientEventProcessor::onPublish()
	{
	}
	void clientEventProcessor::onPubAck()
	{
	}
	void clientEventProcessor::onPubRec()
	{
	}
	void clientEventProcessor::onPubRel()
	{
	}
	void clientEventProcessor::onPubComp()
	{
	}
	void clientEventProcessor::onSubscribe()
	{
	}
	void clientEventProcessor::onUnsubscarube()
	{
	}
	void clientEventProcessor::onPingRsp()
	{
	}
	void clientEventProcessor::onDisconnect()
	{
	}
	int clientEventProcessor::fixedHeaderProcess()
	{
		char Byte;
		switch (_nextPhase)
		{
		case FIXHD_BEGIN:
			{				
				int read_length =:: read(_ownerSock->getSocketHandle(), (void*)&Byte, 1);
				if (read_length == 1)
				{
					_remainLenSectionCount = 0;
					_remaining_length = 0;
					_remaining_mult = 1;
					_command = Byte;
					_nextPhase = FIXHD_REMAIN_LENGTH;
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
						++_remainLenSectionCount;

						if (_remainLenSectionCount > 4) return TINY_SOCKET_ERROR;


						_remaining_length += (Byte & 127) * _remaining_mult;
						_remaining_mult *= 128;
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
				
				_nextPhase = PAYLOAD;	
				if (_payload == NULL)
				{
					_payload = malloc(_remaining_length);
				}			
				else
				{
					free(_payload);
					_payload = malloc(_remaining_length);
				}
			}
		case PAYLOAD:
			{
				int read_length =:: read(_ownerSock->getSocketHandle(),
					(void*)((char *)_payload + _readByteCount), _remaining_length);
				if (read_length == _remaining_length)
				{
					_nextPhase = FIXHD_BEGIN;
					return TINY_SUCCESS;
				}
				else
				{
					if (read_length > 0 && read_length < _remaining_length)
					{
						_readByteCount += read_length;
						_remaining_length -= read_length;
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
}