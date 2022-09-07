#include "networksystem.h"

#include "manager/tcpsocketmanager.h"
#include "function/adapter/kuka_iiwa/kuka_iiwa_encoder.h"

#ifdef _DEBUG
#include <iostream>
#endif // _DEBUG

namespace GComponent {

NetworkSystem& NetworkSystem::getInstance()
{
	static NetworkSystem instance;
	return instance;
}

NetworkSystem::~NetworkSystem() = default;

void NetworkSystem::Initialize() {
	connect(&TcpSocketManager::getInstance(), &TcpSocketManager::RequestSyncOneFrame,
			this,							  &NetworkSystem::ResponseSyncOnce);
}

/*_____________________________________SLOTS METHODS_______________________*/
void NetworkSystem::ResponseLinkClientToServer(const QString& obj_name, QString ip_address, QString port)
{	
	QHostAddress host_ip_address(ip_address);
	quint16		 port_val = port.toUInt();
	bool		 result	  = TcpSocketManager::getInstance().RegisterTcpSocket(
														  obj_name,
														  host_ip_address, 
														  port_val);
	if(!result)  emit NotifySocketLinkError();	
}

void NetworkSystem::ResponseAsyncReceiver(const QString& obj_name, bool flag)
{
	QByteArray msg = flag ? KUKA::Encoder::RequestAsync() : KUKA::Encoder::RequestAsyncStop();
	if (TcpSocketManager::getInstance().TcpSocketWrite(obj_name, msg)){	
#ifdef _DEBUG
		std::cout << "SOCKET ERROR::send failed for some reason\n";
#endif // _DEBUG		
	}
}

void NetworkSystem::ResponseQuit(const QString& obj_name)
{	
	TcpSocketManager::getInstance().DeregisterTcpSocket(obj_name);
}

void NetworkSystem::ResponseHigherAurthority(const QString& obj_name)
{
	QByteArray msg = KUKA::Encoder::RequestController();
	if (TcpSocketManager::getInstance().TcpSocketWrite(obj_name, msg)){
#ifdef _DEBUG
		std::cout << "SOCKET ERROR::send failed for some reason\n";
#endif // _DEBUG		
	}	
}

void NetworkSystem::ResponseSendJointsAngle(const QString& obj_name, std::vector<float> joints) {
	//if (!SafetyCheck()) return;
	QByteArray msg = KUKA::Encoder{}.EncodeMoveSmartServoMotion(joints).GetCommand();
	if(TcpSocketManager::getInstance().TcpSocketWrite(obj_name, msg, false)){
#ifdef _DEBUG
		std::cout << "SOCKET ERROR::send failed for some reason\n";
#endif // _DEBUG		
	}
}

void NetworkSystem::ResponseSyncOnce(const QString& obj_name, int type) {
	QByteArray msg = "GETSYNC";
	switch (type) {
	case 0: msg += "JOINTS";	break;
	case 4: msg += "FORCE";		break;
	case 5: msg += "EXTTORQUE"; break;
	case 2: msg += "POSE";		break;
	}
	msg += "#";
	if (TcpSocketManager::getInstance().TcpSocketWrite(obj_name, msg, false)) {
#ifdef _DEBUG
		std::cout << "SOCKET ERROR::send failed for some reason\n";
#endif // _DEBUG		
	}

}

}


