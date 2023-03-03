/**
 *  @file  	tcpsocketmanager.h
 *  @brief 	This manager responsible for the socket possess.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	9 2th, 2022
 **/
#ifndef __TCPSOCKET_MANAGER_H
#define __TCPSOCKET_MANAGER_H

#include "base/singleton.h"

#include <QtNetwork/QTcpSocket>
#include <QtCore/QObject>

#include <functional>
#include <vector>
#include <memory>
#include <mutex>

namespace GComponent {

class TcpSocketManager : public QObject {
	template<class T> using _Deleter	= std::function<void(T*)>;
	template<class T> using _PtrWithDel = std::unique_ptr<T, _Deleter<T>>;

Q_OBJECT

NonCopyable(TcpSocketManager)

public:
	static TcpSocketManager& getInstance();
	~TcpSocketManager();
	void		Initialize();
	void		tick();
	bool		RegisterTcpSocket	(QString			name, 
									 QHostAddress		address, 
									 unsigned short		port);
	bool		DeregisterTcpSocket	(QString			name);
	QTcpSocket* GetTcpSocket		(QString			name);
	bool		TcpSocketWrite		(const QString&		name, 
									 const QByteArray&	datas,
									 bool				bufferd = true);
protected:
	TcpSocketManager() = default;
	void		SocketConnection	(const QString& name, QTcpSocket* sock);

signals:
	void NotifySocketLinkState		(QString obj_name, bool flag);
	void NotifySocketRank			(QString obj_name, int  rank);
	void NotifyAsyncStatus			(QString obj_name, bool flag);
	void TransmitRobotDatas			(QString obj_name, int  type, std::vector<float> datas);
	void RequestProcessMsg			(QString obj_name);
	void RequestSyncOneFrame	    (const QString& obj_name, int type);	

public slots:	
	void ResponseProcessMsg			(QString obj_name);
	
private:
	std::unordered_map<QString, _PtrWithDel<QTcpSocket>>	socket_table_;
	std::unordered_map<QTcpSocket*, std::mutex>				lock_table_;
};

}

#endif // !__TCPSOCKET_MANAGER_H

