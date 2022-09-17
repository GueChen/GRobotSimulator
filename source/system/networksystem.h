/**
 *  @file  	networksystem.h
 *  @brief 	This class is a bridge to link network inteface and ui.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Sep 1st, 2022
 **/
#ifndef __NETWORK_SYSTEM_H
#define __NETWORK_SYSTEM_H

#include "base/singleton.h"

#include <QtCore/QObject>

#include <vector>

namespace GComponent {

class NetworkSystem : public QObject{
	Q_OBJECT

	NonCoyable(NetworkSystem)

public:
	static NetworkSystem& getInstance();
	~NetworkSystem();
	void Initialize();

signals:
	void NotifySocketLinkError();	

public slots:	
	void ResponseLinkClientToServer(const QString& obj_name, QString ip_address, QString port);
	void ResponseAsyncReceiver	   (const QString& obj_name, bool flag);
	void ResponseQuit			   (const QString& obj_name);
	void ResponseHigherAurthority  (const QString& obj_name);
	void ResponseSendJointsAngle   (const QString& obj_name, std::vector<float> joints);
	void ResponseSendCancelRequest (const QString& obj_name);
	void ResponseSyncOnce		   (const QString& obj_name, int type);


protected:
	NetworkSystem() = default;
};

} // !namespace GComponent

#endif // !__NETWORK_SYSTEM_H

