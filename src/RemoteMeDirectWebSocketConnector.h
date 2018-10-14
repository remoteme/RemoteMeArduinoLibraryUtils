


#include <Arduino.h>

#include <list>

#include <RemoteMeDirectConnector.h>
#include <WebSocketsServer.h>

#ifndef _RemoteMeDirectWebSobsocketConnector_h
#define _RemoteMeDirectWebSobsocketConnector_h


#define LOCAL_SERVER_PORT 80


struct DeviceIdNum {
	uint8_t num;
	uint16_t deviceId;
};

class RemoteMeDirectWebSocketConnector : public RemoteMeDirectConnector
	{
	

	private:
		std::list<DeviceIdNum> deviceIdNum;

		WebSocketsServer* webSocketServer = nullptr;
		static void webSocketServerEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
		

		void removeByNum(uint8_t num);
	protected:
		static RemoteMeDirectWebSocketConnector*  thiz;

		
		std::list<uint16_t>* getDirectConnected();
		void loop();
		void send(uint8_t* payload, uint16_t size);
		bool send(uint16_t deviceId, uint8_t* payload, uint16_t size);

	public:
		RemoteMeDirectWebSocketConnector(void);
	};


#endif //_REMOTEMESocketConnector_h

	