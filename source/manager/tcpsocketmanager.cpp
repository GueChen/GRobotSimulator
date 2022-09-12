#include "manager/tcpsocketmanager.h"

#include "function/adapter/kuka_iiwa/kuka_iiwa_decoder.h"

#ifdef _DEBUG
#include <chrono>
#include <iostream>

#endif // _DEBUG


namespace GComponent {

void TcpSocketCustomDelete(QTcpSocket* socket_ptr);

TcpSocketManager& TcpSocketManager::getInstance()
{
	static TcpSocketManager instance;
	return instance;
}

TcpSocketManager::~TcpSocketManager() = default;

void TcpSocketManager::Initialize()
{
	connect(this,	&TcpSocketManager::RequestProcessMsg, 
			this,	&TcpSocketManager::ResponseProcessMsg);	
}

void TcpSocketManager::tick() {
	for(auto & [name, sock]: socket_table_){
		//emit RequestSyncOneFrame(name, KUKA::DataType::JointCurPos);		
	}
}

bool TcpSocketManager::RegisterTcpSocket(QString name, QHostAddress address, unsigned short port)
{	
	_PtrWithDel<QTcpSocket> socket = _PtrWithDel<QTcpSocket>(new QTcpSocket, TcpSocketCustomDelete);
	SocketConnection(name, socket.get());
	socket->connectToHost(address, port);
	if (!socket->waitForConnected(1000)) return false;	
	DeregisterTcpSocket(name);	
	lock_table_[socket.get()];
	socket_table_.emplace(name, std::move(socket));

	return true;
}

bool TcpSocketManager::DeregisterTcpSocket(QString name)
{
	auto iter = socket_table_.find(name);
	if (iter == socket_table_.end()) return false;

	lock_table_.erase(iter->second.get());
	socket_table_.erase(iter);
	
	return true;
}

QTcpSocket* TcpSocketManager::GetTcpSocket(QString name)
{
	auto iter = socket_table_.find(name);
	if (iter == socket_table_.end()) return nullptr;
	return iter->second.get();
}

bool TcpSocketManager::TcpSocketWrite(const QString& name, const QByteArray& datas, bool buffered) {
	auto iter = socket_table_.find(name);
	if (iter == socket_table_.end()) return false;
	std::lock_guard<std::mutex> lock(lock_table_[iter->second.get()]);
	if (iter->second->write(datas) == -1) {
		// some error information
	}
	else if(!buffered) {
		//iter->second->flush();
	}

#ifdef _DEBUG
	static auto start = std::chrono::steady_clock::now();
	auto cur = std::chrono::steady_clock::now();
	std::cout << "send elapsed: " << std::chrono::duration_cast<std::chrono::milliseconds>(cur - start) << std::endl;
	std::cout << "socket state: " << iter->second->state() << std::endl;
	start = cur;
	//if(iter->second->state() == QAbstractr)
#endif // _DEBUG
	return true;
}

void TcpSocketManager::SocketConnection(const QString& name, QTcpSocket* sock)
{
	connect(sock, &QTcpSocket::readyRead,	 [name, this]() { emit RequestProcessMsg(name); });
	connect(sock, &QTcpSocket::connected,	 [name, this]() { emit NotifySocketLinkState(name, true); });
	connect(sock, &QTcpSocket::disconnected, [name, this]() { emit NotifySocketLinkState(name, false); });
#ifdef _DEBUG
	connect(sock, &QTcpSocket::errorOccurred, [this, sock, name](QAbstractSocket::SocketError err) {		
		std::cout <<"Error happend: " << err << ", try to reconnect: " << 
			sock->localAddress().toString().toStdString() << "->" << sock->peerPort() << std::endl;
		//RegisterTcpSocket(name, sock->peerAddress(), sock->peerPort());
		DeregisterTcpSocket(name);
		});
#endif // _DEBUG
	
}

/*______________________________SLOTS METHODS________________________________*/
void TcpSocketManager::ResponseProcessMsg(QString obj_name) {
	
	
	QTcpSocket*	   socket = GetTcpSocket(obj_name);
	if (socket == nullptr) return;
	QByteArray	   datas  = socket->readAll();
	QByteArrayList msgs	  = datas.split('#');

#ifdef _DEBUG
	int data_count = 0, msg_count = 0, async_count = 0;
#endif // _DEBUG

	for (auto& msg : msgs) {
		KUKA::Decoder  decoder(msg.data(), msg.size());
		if (decoder.IsMessageType()) {
			emit NotifySocketRank(obj_name, decoder.GetRankInfo());
#ifdef _DEBUG
			++msg_count;
			//std::cout << "msg type: " << msg.toHex(' ') << std::endl;
#endif
		}else if (decoder.IsAsyncStatus()) {
			emit NotifyAsyncStatus(obj_name, decoder.GetAsyncInfo());
#ifdef _DEBUG
			++async_count;
			//std::cout << "async status: " << msg.toHex(' ') << std::endl;
#endif
		}else{
			KUKA::DataType type = decoder.GetType();
			if (type == KUKA::DataType::None) continue;
			emit TransmitRobotDatas(obj_name, type, decoder.GetDatas());
#ifdef _DEBUG
			++data_count;
			//std::cout << "data type: " << msg.toHex(' ') << std::endl;
#endif
		}		
	}	

#ifdef _DEBUG
	static auto start = std::chrono::steady_clock::now();
	auto cur = std::chrono::steady_clock::now();	
	std::cout << "receive elapsed: " << std::chrono::duration_cast<std::chrono::milliseconds>(cur - start)
			  << ", receive " << data_count    << " data, " << 
								   msg_count   << " msg, "  <<
								   async_count << " async"<<std::endl;
	start = cur;
#endif // _DEBUG
	
}

void TcpSocketCustomDelete(QTcpSocket* socket_ptr)  {
	static QByteArray quit_cmd = QByteArray("quit#");
	if (socket_ptr->state() == QTcpSocket::ConnectedState && socket_ptr->isWritable()) {
		socket_ptr->write(quit_cmd);
	}
	if (socket_ptr->state() != QTcpSocket::UnconnectedState) {
		socket_ptr->disconnectFromHost();
		socket_ptr->waitForDisconnected();
	}
	socket_ptr->close();
	delete socket_ptr;
	socket_ptr = nullptr;
}
}