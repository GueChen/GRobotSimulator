/**
 *  @file  	transmitsystem.h
 *  @brief 	This class is responsible for the data communications.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Sep 4th, 2022
 **/

#ifndef __TRANSMIT_SYSTEM_H
#define __TRANSMIT_SYSTEM_H

#include "base/singleton.h"

#include <QtCore/QObject>

#include <vector>

namespace GComponent {

class TransmitSystem : public QObject {
	enum Mode {
		eNormal = 0, eV2R, eR2V
	};

	Q_OBJECT

	NonCoyable(TransmitSystem)

public:
	static TransmitSystem& getInstance();
	~TransmitSystem();

protected:
	TransmitSystem() = default;

signals:
	void SendPlanningDatas(const QString& obj_name, std::vector<float> joints);

public slots:	
	void ResponseModeChange(const QString& obj_name, int mode);
	void ReceiveJointsAngle(const QString& obj_name, std::vector<float> joints);
	void ProcessRobotTransmitDatas
						   (const QString& obj_name, int type, std::vector<float> datas);
protected:
	Mode left_transmit_mode_ = eNormal;
};

}

#endif // !__TRANSMIT_SYSTEM_H
