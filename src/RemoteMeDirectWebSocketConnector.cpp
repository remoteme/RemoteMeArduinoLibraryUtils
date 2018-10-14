// 
// 
// 

#include "RemoteMeDirectWebSocketConnector.h"

RemoteMeDirectWebSocketConnector*  RemoteMeDirectWebSocketConnector::thiz;

RemoteMeDirectWebSocketConnector::RemoteMeDirectWebSocketConnector(void) {
	RemoteMeDirectWebSocketConnector::thiz = this;
	webSocketServer = new WebSocketsServer(LOCAL_SERVER_PORT);
	webSocketServer->begin();
	webSocketServer->onEvent(RemoteMeDirectWebSocketConnector::webSocketServerEvent);
}


void RemoteMeDirectWebSocketConnector::webSocketServerEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
		static bool zm = true;

		switch (type) {
		case WStype_DISCONNECTED:
			RemoteMeDirectWebSocketConnector::thiz->removeByNum(num);
			DEBUG_REMOTEME("[RMM] disconnected direct num :%d total connected %d\n", num, RemoteMeDirectWebSocketConnector::thiz->deviceIdNum.size());

			break;
		case WStype_CONNECTED:
			DEBUG_REMOTEME("[RMM] connected direct num : %d  total connected %d\n", num, RemoteMeDirectWebSocketConnector::thiz->deviceIdNum.size());
			break;
		case WStype_TEXT: {
			char ret[length + 1];
			memcpy(ret, payload, length);
			ret[length] = 0;

			String str(ret);
			
			DeviceIdNum d;
			d.deviceId= str.toInt();
			d.num = num;
			RemoteMeDirectWebSocketConnector::thiz->deviceIdNum.push_back(d);
			DEBUG_REMOTEME("[RMM] registeres direct num :%d  deviceId:%d all direct conencted: \n",d.num, d.deviceId, RemoteMeDirectWebSocketConnector::thiz->deviceIdNum.size());

			// send message to server
			// webSocket.sendTXT("message here");
			break;
		}
		
		case WStype_BIN:
			DEBUG_REMOTEME("[RMM] direct bin message came length:%d \n",length);
			if (length > 0) {
				RemoteMeDirectWebSocketConnector::thiz->processMessage(payload);
			}
		}
		
	}
	

void RemoteMeDirectWebSocketConnector::removeByNum(uint8_t num) {
	for (std::list<DeviceIdNum>::iterator it = this->deviceIdNum.begin(); it != this->deviceIdNum.end(); ++it) {
		if ((*it).num==num) {
			it = this->deviceIdNum.erase(it);
			return;
		}
	}
}
void RemoteMeDirectWebSocketConnector::send(uint8_t * payload,uint16_t size ) {
	webSocketServer->broadcastBIN(payload, size);
	DEBUG_REMOTEME("[RMM] Broadcasting bin message to %d\n",this->deviceIdNum.size());

}

bool RemoteMeDirectWebSocketConnector::send(uint16_t deviceId,uint8_t * payload, uint16_t size) {
	DEBUG_REMOTEME("[RMM] Sending direct binary message \n");
	for (std::list<DeviceIdNum>::iterator it = this->deviceIdNum.begin(); it != this->deviceIdNum.end(); ++it) {
		if ((*it).deviceId == deviceId) {
		
			webSocketServer->sendBIN((*it).num,payload, size);
			DEBUG_REMOTEME("[RMM] sent direct binary message to %d \n", deviceId);
			return true;
		}
	}
	return false;
}

std::list<uint16_t>* RemoteMeDirectWebSocketConnector::getDirectConnected() {
	std::list<uint16_t>* ret = new std::list<uint16_t>;
	for (std::list<DeviceIdNum>::iterator it = this->deviceIdNum.begin(); it != this->deviceIdNum.end(); ++it) {
		ret->push_back((*it).deviceId);
	}
	return ret;
}


void RemoteMeDirectWebSocketConnector::loop() {
	if (webSocketServer!=nullptr) {
		webSocketServer->loop();
	}
}



	
