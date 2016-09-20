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
		willMessagePublish();
		if (_packet != NULL)delete _packet;
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
		delete _packet;
		_packet = NULL;
		this->updateVisitTime();
		return true;
	}
	bool clientEventProcessor::handleWriteEvent()
	{
		bool saveEvent = false;
		while (!_session->_messgaeSendQueue.empty())
		{

			std::shared_ptr<tinymq::tinyPacket> tp = _session->_messgaeSendQueue.front();
			_session->_messgaeSendQueue.pop();

			if ((tp->_command & 0xF0) == PUBLISH)
			{
				if ((tp->_command & 0x06) >> 1 == 1)
				{
					if (_session->waitingPubAckMap.find(tp->mid) == _session->waitingPubAckMap.end())
					{
						_session->waitingPubAckMap.insert(std::make_pair(tp->mid, tp));
						_session->midsInuse.insert(tp->mid);
					}
					else
					{
						return true;
					}
				}

				if ((tp->_command & 0x06) >> 1 == 2)
				{
					if (_session->waitingPubRecMap.find(tp->mid) == _session->waitingPubRecMap.end())
					{
						_session->waitingPubRecMap.insert(std::make_pair(tp->mid, tp));
						_session->midsInuse.insert(tp->mid);
					}
					else
					{
						return true;
					}
				}
			}
	
			int rc = tinyPacketWrite(tp);
			if (rc == TINY_ERROR)
			{
				closeAndClearSession();
				return false;
			}
			else if (rc == TINY_EAGAIN) {
				_writeOccurred = false;
				_session->_messgaeSendQueue.push(tp);
				return true;
			}
		}
		_ownerSock->makeSocketDisableWrite();
		_writeOccurred = false;
		updateVisitTime();
		return true;
	}
	int clientEventProcessor::tinyPacketWrite(std::shared_ptr<tinymq::tinyPacket>tp)
	{
		assert(tp);
		//signal(SIGPIPE, SIG_IGN);
		int writeLength = write(_ownerSock->getSocketHandle(), (void*)(tp->_payload + tp->_payloadPos), tp->_payloadLen - tp->_payloadPos);
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

		default:

			return 0;
		}
	}
	int clientEventProcessor::onConnect()
	{
		tinyServer::instance()->removeWaittingSock(this);
		char sessionPresent = 0x00;// connectAckÖÐ»áÓÃµ½
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
		if (readByte(&protocol_version) != TINY_SUCCESS)
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

		if (readByte(&flag) != TINY_SUCCESS)
		{
			goto tiny_erro_occur;
		}
		if (readShort(keepAlive) != TINY_SUCCESS)
		{
			goto tiny_erro_occur;
		}
		if (readString(&clientId) != TINY_SUCCESS || clientId == NULL || strlen(clientId) == 0)
		{
			onConnectAck(CONNACK_REFUSED_IDENTIFIER_REJECTED, sessionPresent);
			goto tiny_erro_occur;
		}
		
		clientidStr.append(clientId);
		std::cout << "recive connection of client_id: " << clientidStr << std::endl;

		_keepAlive = keepAlive;

		if (flag & 0x01)
		{
			goto tiny_erro_occur;
		}

		if (flag & 0x02) {
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


		if (flag & 0x04) {
			willFlag = true;
			willQos = (flag & 0x18) >> 3;
			if (willQos == 3) goto tiny_erro_occur;

			std::unique_ptr<willMessage> willmsg(new willMessage());

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
			willmsg->_willPayloadLen = willPayloadLen;
			willPayload = (char*)malloc(willPayloadLen);
			readBytes(willPayload, willPayloadLen);
			willmsg->_willPayload = willPayload;
			_session->setWillMsg(willmsg.get());
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

		rc = onConnectAck(CONNACK_ACCEPTED, sessionPresent);
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
		std::shared_ptr<tinymq::tinyPacket> tp(new tinyPacket());
		tp->_command = CONNACK;
		tp->_remaining_length = 2;
		tinyPacketAlloc(tp);
		tp->_payload[tp->_payloadPos] = sessionPre;
		tp->_payloadPos++;
		tp->_payload[tp->_payloadPos] = backCode;
		tp->_payloadPos = 0;
		_session->_messgaeSendQueue.push(tp);
		_ownerSock->makeSocketEnableWrite();
		return TINY_SUCCESS;
	}
	int clientEventProcessor::onPublish()
	{

		char* topic = NULL;

		char qos = (_packet->_command & 0x06) >> 1;
		char dup = (_packet->_command & 0x8) >> 3;
		short mid = 0;
		if (readString(&topic) != TINY_SUCCESS || topic == NULL)
		{

			return TINY_ERROR;
		}
		std::string topicStr(topic);
		if (qos > 0) {
			if (readShort(mid) != TINY_SUCCESS)
			{
				free(topic);
				return TINY_ERROR;
			}
		}
		if (_packet->_payloadLen - _packet->_payloadPos == 0)
		{
			free(topic);
			return TINY_SUCCESS;
		}

		int payloadlen = _packet->_payloadLen - _packet->_payloadPos;
		char* payload = (char *)malloc(payloadlen);
		if (payload == NULL)
		{
			free(topic);
			return TINY_ERROR;
		}
		readBytes(payload, payloadlen);

		if (1 == qos)
		{
			sendPublishAck(mid);
		}
		else if (2 == qos)
		{
			sendPublishRec(mid);
		}
		if (qos <2)
			publishToTopic(topicStr, payload, payloadlen, mid);
		else if (2 == qos && _session->midsInuse.find(mid) == _session->midsInuse.end())
		{
			publishToTopic(topicStr, payload, payloadlen, mid);
			_session->midsInuse.insert(mid);
		}

		free(payload);
		free(topic);
		return TINY_SUCCESS;
	}
	int clientEventProcessor::onPubAck()
	{
		short mid = 0;
		if (readShort(mid) != TINY_SUCCESS)
		{
			return TINY_ERROR;
		}
		auto it = _session->waitingPubAckMap.find(mid);
		if (it != _session->waitingPubAckMap.end())
		{
			_session->waitingPubAckMap.erase(it);
			_session->midsInuse.erase(mid);
		}
		
	}
	int clientEventProcessor::onPubRec()
	{
		short mid = 0;
		if (readShort(mid) != TINY_SUCCESS)
		{
			return TINY_ERROR;
		}
		auto it = _session->waitingPubRecMap.find(mid);
		if (it != _session->waitingPubRecMap.end())
		{
			_session->waitingPubRecMap.erase(it);
		}	
		sendPubRel(mid);
		return TINY_SUCCESS;
	}
	int clientEventProcessor::sendPubRel(short mid)
	{
		std::shared_ptr<tinymq::tinyPacket> tp(new tinyPacket());
		tp->sendTime = time(NULL);
		tp->_command = PUBREL;
		tp->_remaining_length = 2;
		tinyPacketAlloc(tp);
		char msb, lsb;
		msb = (mid & 0xFF00) >> 8;
		lsb = mid & 0x00FF;
		tp->_payload[tp->_payloadPos] = msb;
		tp->_payloadPos++;
		tp->_payload[tp->_payloadPos] = lsb;
		tp->_payloadPos++;
		tp->_payloadPos = 0;
		_session->_messgaeSendQueue.push(tp);
		_session->waitingPubCompMap.insert(std::make_pair(mid,tp));
		_ownerSock->makeSocketEnableWrite();
		return TINY_SUCCESS;
	}
	int clientEventProcessor::onPubRel()
	{
		short mid = 0;
		if (readShort(mid) != TINY_SUCCESS)
		{
			return TINY_ERROR;
		}
		auto it = _session->midsInuse.find(mid);
		if (it != _session->midsInuse.end())
		{
			_session->midsInuse.erase(it);
		}
		sendPubComp(mid);
		return TINY_SUCCESS;
	}
	int clientEventProcessor::sendPubComp(short mid)
	{
		std::shared_ptr<tinymq::tinyPacket> tp(new tinyPacket());
		tp->_command = PUBCOMP;
		tp->_remaining_length = 2;
		tinyPacketAlloc(tp);
		char msb, lsb;
		msb = (mid & 0xFF00) >> 8;
		lsb = mid & 0x00FF;
		tp->_payload[tp->_payloadPos] = msb;
		tp->_payloadPos++;
		tp->_payload[tp->_payloadPos] = lsb;
		tp->_payloadPos++;
		tp->_payloadPos = 0;
		_session->_messgaeSendQueue.push(tp);
		_ownerSock->makeSocketEnableWrite();
		return TINY_SUCCESS;
	}

	int clientEventProcessor::onPubComp()
	{

		short mid = 0;
		if (readShort(mid) != TINY_SUCCESS)
		{
			return TINY_ERROR;
		}
		auto itrel = _session->waitingPubCompMap.find(mid);
		if (itrel != _session->waitingPubCompMap.end())
		{
			_session->waitingPubCompMap.erase(itrel);
		}
		auto it = _session->midsInuse.find(mid);
		if (it != _session->midsInuse.end())
		{
			_session->midsInuse.erase(it);
		}
		return TINY_SUCCESS;
	}
	int clientEventProcessor::onSubscribe()
	{
		std::vector<char> result;
		short mid = 0;
		char qos;
		char* topic;
		if (_packet->_command & 0x0F != 0x02)
			return TINY_ERROR;
		if (readShort(mid) != TINY_SUCCESS)
		{
			return TINY_ERROR;
		}
		while (_packet->_payloadPos < _packet->_remaining_length)
		{
			if (readString(&topic) != TINY_SUCCESS || topic == NULL)
			{
				return TINY_ERROR;
			}
			if (readByte(&qos) != TINY_SUCCESS || qos>2)
			{
				free(topic);
				return TINY_ERROR;
			}
			std::string topicStr(topic);
			free(topic);
			_session->topicSubscribed.insert(topicStr);
			tinyServer::instance()->addTopicSubscriber(topicStr, this->_session, qos);
			if (qos == 0)
				result.push_back(0x00);
			else if (qos == 1)
				result.push_back(0x01);
			else
				result.push_back(0x02);

		}

		return sendSubscribeAck(result, mid);

	}
	int clientEventProcessor::sendSubscribeAck(std::vector<char>& vc, short mid)
	{
		std::shared_ptr<tinymq::tinyPacket> tp(new tinyPacket());
		char msb, lsb;
		msb = (mid & 0xFF00) >> 8;
		lsb = mid & 0x00FF;
		tp->_command = SUBACK;
		tp->_remaining_length = 2 + vc.size();
		tinyPacketAlloc(tp);
		tp->_payload[tp->_payloadPos] = msb;
		tp->_payloadPos++;
		tp->_payload[tp->_payloadPos] = lsb;
		tp->_payloadPos++;
		for (auto it : vc)
		{
			tp->_payload[tp->_payloadPos] = it;
			tp->_payloadPos++;
		}
		tp->_payloadPos = 0;
		_session->_messgaeSendQueue.push(tp);
		_ownerSock->makeSocketEnableWrite();
		return TINY_SUCCESS;
	}
	int clientEventProcessor::onUnsubscarube()
	{
		char* topic = NULL;
		if (_packet->_command & 0x0F != 0x02)
			return TINY_ERROR;
		short mid;
		if (readShort(mid) != TINY_SUCCESS)
		{
			return TINY_ERROR;
		}
		if (readString(&topic) != TINY_SUCCESS || topic == NULL)
		{

			return TINY_ERROR;
		}
		std::string topicStr = std::string(topic);
		if (tinyServer::instance()->_topicSubscriber[topicStr] != NULL)
		{
			std::map<tinySession*, char>* tempMap =tinyServer::instance()->_topicSubscriber[topicStr];
			tempMap->erase(_session);
		}
		_session->topicSubscribed.erase(topicStr);
		return sendUnsubscarubeAck(mid);
	}
	int clientEventProcessor::sendUnsubscarubeAck(short mid)
	{
		std::shared_ptr<tinymq::tinyPacket> tp(new tinyPacket());
		tp->_command = UNSUBACK;
		tp->_remaining_length = 2;
		tinyPacketAlloc(tp);
		char msb, lsb;
		msb = (mid & 0xFF00) >> 8;
		lsb = mid & 0x00FF;
		tp->_payload[tp->_payloadPos] = msb;
		tp->_payloadPos++;
		tp->_payload[tp->_payloadPos] = lsb;
		tp->_payloadPos++;
		tp->_payloadPos = 0;
		_session->_messgaeSendQueue.push(tp);
		_ownerSock->makeSocketEnableWrite();
		return TINY_SUCCESS;
	}
	int clientEventProcessor::onPingRsp()
	{
		std::shared_ptr<tinymq::tinyPacket> tp(new tinyPacket());
		tp->_command = PINGRESP;
		tp->_remaining_length = 0;
		tinyPacketAlloc(tp);
		tp->_payloadPos = 0;
		_session->_messgaeSendQueue.push(tp);
		_ownerSock->makeSocketEnableWrite();
		return TINY_SUCCESS;
	}
	int clientEventProcessor::onDisconnect()
	{
		return TINY_ERROR;
	}
	int clientEventProcessor::resendPublish(std::shared_ptr<tinymq::tinyPacket> tp)
	{
		tp->sendTime = time(NULL);
		tp->_payload[0] = tp->_payload[0] & 0x08;
		acceptPublish(tp);
	}
	int clientEventProcessor::resendPubRel(std::shared_ptr<tinymq::tinyPacket> tp)
	{
		acceptPublish(tp);
	}
	int clientEventProcessor::sendPublishAck(short mid)
	{
		std::shared_ptr<tinymq::tinyPacket> tp(new tinyPacket());
		tp->_command = PUBACK;
		tp->_remaining_length = 2;
		tinyPacketAlloc(tp);
		char msb, lsb;
		msb = (mid & 0xFF00) >> 8;
		lsb = mid & 0x00FF;
		tp->_payload[tp->_payloadPos] = msb;
		tp->_payloadPos++;
		tp->_payload[tp->_payloadPos] = lsb;
		tp->_payloadPos++;
		tp->_payloadPos = 0;
		_session->_messgaeSendQueue.push(tp);
		_ownerSock->makeSocketEnableWrite();
		return TINY_SUCCESS;
	}
	int clientEventProcessor::sendPublishRec(short mid)
	{
	
		std::shared_ptr<tinymq::tinyPacket> tp(new tinyPacket());
		tp->_command = PUBREC;
		tp->_remaining_length = 2;
		tinyPacketAlloc(tp);
		char msb, lsb;
		msb = (mid & 0xFF00) >> 8;
		lsb = mid & 0x00FF;
		tp->_payload[tp->_payloadPos] = msb;
		tp->_payloadPos++;
		tp->_payload[tp->_payloadPos] = lsb;
		tp->_payloadPos++;
		tp->_payloadPos = 0;
		_session->_messgaeSendQueue.push(tp);
		_ownerSock->makeSocketEnableWrite();
		return TINY_SUCCESS;
	}
	int clientEventProcessor::publishToTopic(std::string& topic, char* payload, int payloadLen, short mid)
	{
		/*
		由于std::string没有splite函数，故手动以 '/’为区分符号划分topic等级
		*/
		char qos = (_packet->_command & 0x06) >> 1;
		char dup = (_packet->_command & 0x8) >> 3;
		char retain = _packet->_command & 0x01;
		short topicLen = topic.length();
		char msb, lsb;

		if (tinyServer::instance()->_topicSubscriber["#"] != NULL)
			for (auto it : *(tinyServer::instance()->_topicSubscriber["#"]))
			{
				//if (it.second < qos)
				qos = it.second < qos ? it.second :qos;
				sendPublishPayload(topic, payload, payloadLen, mid, qos,  it.first);

			}

		size_t last = 0;
		size_t index = topic.find_first_of("/", last);
		std::string resultStr;
		std::string tmp_str = topic.substr(last, index - last);
		if (tmp_str.compare("") == 0)
		{
			last = index + 1;
			index = topic.find_first_of("/", last);
			resultStr.append("/").append(topic.substr(last, index - last));
			//_addTopic(resultStr + "/#", cep);
			if (tinyServer::instance()->_topicSubscriber[resultStr + "/#"] != NULL)
				for (auto it : *(tinyServer::instance()->_topicSubscriber[resultStr + "/#"]))
				{
					qos = it.second < qos ? it.second : qos;
					sendPublishPayload(topic, payload, payloadLen, mid, qos, it.first);
				}
		}
		else {
			resultStr = tmp_str;
			//_addTopic(resultStr + "/#", cep);
			if (tinyServer::instance()->_topicSubscriber[resultStr + "/#"] != NULL)
				for (auto it : *(tinyServer::instance()->_topicSubscriber[resultStr + "/#"]))
				{
					qos = it.second < qos ? it.second : qos;
					sendPublishPayload(topic, payload, payloadLen, mid, qos,it.first);
				}
		}

		while (index != -1)
		{
			last = index + 1;
			index = topic.find_first_of("/", last);
			std::string tmp_str = topic.substr(last, index - last);
			resultStr.append("/").append(tmp_str);
			if (index == -1 && tmp_str.compare("") == 0)
				break;
			else
			{
				//_addTopic(resultStr + "/#", cep);
				if (tinyServer::instance()->_topicSubscriber[resultStr + "/#"] != NULL)
					for (auto it : *(tinyServer::instance()->_topicSubscriber[resultStr + "/#"]))
					{
						qos = it.second < qos ? it.second : qos;
						sendPublishPayload(topic, payload, payloadLen, mid, qos, it.first);
					}
			}

		}

		if (tinyServer::instance()->_topicSubscriber[topic] != NULL)
			for (auto it : *(tinyServer::instance()->_topicSubscriber[topic]))
			{
				qos = it.second < qos ? it.second : qos;
				sendPublishPayload(topic, payload, payloadLen, mid, qos, it.first);
			}
		return TINY_SUCCESS;
	}
	int clientEventProcessor::acceptPublish(std::shared_ptr<tinymq::tinyPacket> tp)
	{
		_session->_messgaeSendQueue.push(tp);
		_ownerSock->makeSocketEnableWrite();
		return TINY_SUCCESS;
	}
	int clientEventProcessor::fixedHeaderProcess()
	{
		char Byte;

		switch (_packet->_nextPhase)
		{
		case FIXHD_BEGIN:
		{
			int read_length = ::read(_ownerSock->getSocketHandle(), (void*)&Byte, 1);
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
				int read_length = ::read(_ownerSock->getSocketHandle(), (void*)&Byte, 1);
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
			_packet->_payloadLen = _packet->_remaining_length;//收到的payload，不包括固定报头
		}
		case PAYLOAD:
		{
			int read_length = ::read(_ownerSock->getSocketHandle(),
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

		*str = (char*)malloc(len + 1);
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
	int clientEventProcessor::tinyPacketAlloc(std::shared_ptr<tinymq::tinyPacket> tp)
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
			if (remainLength >0) {
				byte = byte | 0x80;
			}
			remainByte[tp->_remainLenSectionCount] = byte;
			tp->_remainLenSectionCount++;
		} while (remainLength > 0 && tp->_remainLenSectionCount < 5);

		if (tp->_remainLenSectionCount == 5)  return TINY_ERROR;
		tp->_payloadLen = tp->_remainLenSectionCount + 1 + tp->_remaining_length;

		tp->_payload = (char *)malloc(tp->_payloadLen);
		if (tp->_payload == NULL) return TINY_ERROR;

		tp->_payload[0] = tp->_command;
		for (int i = 0; i < tp->_remainLenSectionCount; i++) {
			tp->_payload[i + 1] = remainByte[i];
		}
		tp->_payloadPos = 1 + tp->_remainLenSectionCount;


		return TINY_SUCCESS;
	}
	int clientEventProcessor::sendPublishPayload(std::string& topic, char* payload, int payloadLen, short mid, char qos, tinySession* tSession)
	{
		char dup = (_packet->_command & 0x8) >> 3;
		char retain = _packet->_command & 0x01;
		std::shared_ptr<tinymq::tinyPacket> tp(new tinyPacket());
		tp->sendTime = time(NULL);
		tp->mid = mid;
		tp->_command = PUBLISH | ((dup & 0x1) << 3) | (qos << 1) | retain;
		if (qos > 0)
			tp->_remaining_length = 2 + topic.length() + 2 + payloadLen;
		else
			tp->_remaining_length = 2 + topic.length() + payloadLen;


		tinyPacketAlloc(tp);
		char msb = (topic.length() & 0xFF00) >> 8;
		char lsb = topic.length() & 0x00FF;

		tp->_payload[tp->_payloadPos] = msb;
		tp->_payloadPos++;
		tp->_payload[tp->_payloadPos] = lsb;
		tp->_payloadPos++;

		memcpy(&tp->_payload[tp->_payloadPos], topic.c_str(), topic.length());
		tp->_payloadPos += topic.length();

		if (qos > 0)
		{
			msb = (mid & 0xFF00) >> 8;
			lsb = mid & 0x00FF;
			tp->_payload[tp->_payloadPos] = msb;
			tp->_payloadPos++;
			tp->_payload[tp->_payloadPos] = lsb;
			tp->_payloadPos++;
		}
		memcpy(&tp->_payload[tp->_payloadPos], payload, payloadLen);
		tp->_payloadPos = 0;
		if(tSession!= NULL)
			tSession->getClientEP()->acceptPublish(tp);
	}
	bool clientEventProcessor::isOvertime()
	{
		time_t tmp = time(NULL);
		if (tmp > (_lastVisitTime + _keepAlive*1.5))//超出规定保持连接时间的1.5倍
			return true;
		else
			return false;
	}
	void clientEventProcessor::updateVisitTime()
	{
		_lastVisitTime = time(NULL);
	}
	int clientEventProcessor::willMessagePublish()
	{
		if (_session->getWillMsg()._willPayloadLen == -1)
			return TINY_SUCCESS;
		char qos = 0;
		if (tinyServer::instance()->_topicSubscriber["#"] != NULL)
			for (auto it : *(tinyServer::instance()->_topicSubscriber["#"]))
			{
				sendPublishPayload(_session->getWillMsg()._willTopic, _session->getWillMsg()._willPayload, _session->getWillMsg()._willPayloadLen,0,qos,it.first);
			}

		size_t last = 0;
		size_t index = _session->getWillMsg()._willTopic.find_first_of("/", last);
		std::string resultStr;
		std::string tmp_str = _session->getWillMsg()._willTopic.substr(last, index - last);
		if (tmp_str.compare("") == 0)
		{
			last = index + 1;
			index = _session->getWillMsg()._willTopic.find_first_of("/", last);
			resultStr.append("/").append(_session->getWillMsg()._willTopic.substr(last, index - last));
			//_addTopic(resultStr + "/#", cep);
			if (tinyServer::instance()->_topicSubscriber[resultStr + "/#"] != NULL)
				for (auto it : *(tinyServer::instance()->_topicSubscriber[resultStr + "/#"]))
				{
					sendPublishPayload(_session->getWillMsg()._willTopic, _session->getWillMsg()._willPayload, _session->getWillMsg()._willPayloadLen, 0, qos, it.first);
				}
		}
		else {
			resultStr = tmp_str;
			//_addTopic(resultStr + "/#", cep);
			if (tinyServer::instance()->_topicSubscriber[resultStr + "/#"] != NULL)
				for (auto it : *(tinyServer::instance()->_topicSubscriber[resultStr + "/#"]))
				{
					sendPublishPayload(_session->getWillMsg()._willTopic, _session->getWillMsg()._willPayload, _session->getWillMsg()._willPayloadLen, 0, qos, it.first);
				}
		}

		while (index != -1)
		{
			last = index + 1;
			index = _session->getWillMsg()._willTopic.find_first_of("/", last);
			std::string tmp_str = _session->getWillMsg()._willTopic.substr(last, index - last);
			resultStr.append("/").append(tmp_str);
			if (index == -1 && tmp_str.compare("") == 0)
				break;
			else
			{
				//_addTopic(resultStr + "/#", cep);
				if (tinyServer::instance()->_topicSubscriber[resultStr + "/#"] != NULL)
					for (auto it : *(tinyServer::instance()->_topicSubscriber[resultStr + "/#"]))
					{
						sendPublishPayload(_session->getWillMsg()._willTopic, _session->getWillMsg()._willPayload, _session->getWillMsg()._willPayloadLen, 0, qos, it.first);
					}
			}

		}

		if (tinyServer::instance()->_topicSubscriber[_session->getWillMsg()._willTopic] != NULL)
			for (auto it : *(tinyServer::instance()->_topicSubscriber[_session->getWillMsg()._willTopic]))
			{
				sendPublishPayload(_session->getWillMsg()._willTopic, _session->getWillMsg()._willPayload, _session->getWillMsg()._willPayloadLen, 0, qos, it.first);
			}
		return TINY_SUCCESS;
	}
}