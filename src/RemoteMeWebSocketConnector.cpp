// 
// 
// 

#include "RemoteMeWebSocketConnector.h"

RemoteMeWebSocketConnector*  RemoteMeWebSocketConnector::thiz;

RemoteMeWebSocketConnector::RemoteMeWebSocketConnector(void) {
	RemoteMeWebSocketConnector::thiz = this;
}
String  RemoteMeWebSocketConnector::getIp() {
	return WiFi.localIP().toString();
}


	void RemoteMeWebSocketConnector::send(uint8_t * payload,uint16_t size ) {
		if (webSocketConnected) {
			webSocket->sendBIN(payload, size);
		}
	}

	

	void RemoteMeWebSocketConnector::ping() {
		if (webSocketConnected) {
			if (!webSocket->sendPing()) {
				webSocketConnected = false;
			}
			else {
				
				uint8_t *buffer = (uint8_t*)malloc(4);
				buffer[0] = 0;
				buffer[1] = 0;
				buffer[2] = 0;
				buffer[3] = 0;
				send(buffer, 4);
				free(buffer);
				DEBUG_REMOTEME("[RMM] ping send\n");
			}
		}
	}

	
	void RemoteMeWebSocketConnector::waitForConnection() {
		static unsigned long lastTimePing = 0;

		if (lastTimePing + PING_SEND < deltaMillis() && webSocketConnected) {
			ping();
			lastTimePing = deltaMillis();
		}
		if (!webSocketConnected || (lastTimePingReceived + PING_RECEIVE_TIMEOUT < deltaMillis()))  {
		
			DEBUG_REMOTEME("[RMM] not connected or didnt got ping webSocketConnected: %d last time ping received \n",webSocketConnected,(lastTimePingReceived + PING_RECEIVE_TIMEOUT < deltaMillis()));
			
	
				
			webSocketConnected = false;
			if (webSocket != nullptr) {
				webSocket->disconnect();
				delete webSocket;
			}
			
			webSocket = new WebSocketsClient();

			char* buf = new char[20];
			snprintf(buf, 20, "/api/ws/v1/%d", getDeviceId());

			webSocket->begin(REMOTEME_HOST, REMOTEME_HTTP_PORT, buf, "as");
			webSocket->setAuthorization("token", getToken()); // HTTP Basic Authorization

			webSocket->setReconnectInterval(500);
			webSocket->onEvent(RemoteMeWebSocketConnector::webSocketEvent);
			lastTimePingReceived = deltaMillis() + PING_RECEIVE_TIMEOUT;
			int continousReconnect = 0;
			while (!webSocketConnected) {
				if (continousReconnect++>3000){//5min
					DEBUG_REMOTEME("[RMM] restarting\n");
					ESP.restart();
				}
				webSocket->loop();
				DEBUG_REMOTEME("[RMM] attemp connect\n");
				delay(100);
			}
			DEBUG_REMOTEME("[RMM] connected\n");
			sendVariableObserveMessage();
		}
		
		
	}

	void RemoteMeWebSocketConnector::webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
		static bool zm = true;

		switch (type) {
		case WStype_DISCONNECTED:
			DEBUG_REMOTEME("[RMM] disconnected web socket\n");
			RemoteMeWebSocketConnector::thiz->webSocketConnected = false;
			break;
		case WStype_CONNECTED:
			DEBUG_REMOTEME("[RMM] connected websocket\n");
			RemoteMeWebSocketConnector::thiz->webSocketConnected = true;
			RemoteMeWebSocketConnector::thiz->webSocket->setReconnectInterval(20000);

			break;
		case WStype_TEXT:

			// send message to server
			// webSocket.sendTXT("message here");
			break;
		case WStype_BIN:
			if (length > 0) {
				if ((payload[0]==0)&&(payload[1]==0)){
					DEBUG_REMOTEME("[RMM] ping received\n");
					RemoteMeWebSocketConnector::thiz->lastTimePingReceived = RemoteMeWebSocketConnector::thiz->deltaMillis();
				}else{
					RemoteMeWebSocketConnector::thiz->processMessage(payload);
				}
				
			}

		}


	}

	void RemoteMeWebSocketConnector::loop() {
		if (webSocket!=nullptr) {
			webSocket->loop();
		}
	}
	void RemoteMeWebSocketConnector::disconnect() {
		
		if (webSocket != nullptr) {
			webSocket->disconnect();
			delete webSocket;
			webSocket = nullptr;
		}
	}
	

	
