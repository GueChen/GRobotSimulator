#include "system/transmitsystem.h"
#include "system/networksystem.h"

#include "manager/modelmanager.h"

#include "function/adapter/kuka_iiwa/kuka_iiwa_decoder.h"
#include "component/joint_group_component.h"

#include <map>

namespace GComponent {

static std::map<QString, QString> obj_map = {
	{"left",  "kuka_iiwa_robot_0"},
	{"right", "kuka_iiwa_robot_1"}
};

TransmitSystem& TransmitSystem::getInstance() {
	static TransmitSystem instance;
	return instance;
}

TransmitSystem::~TransmitSystem() = default;

void TransmitSystem::ResponseModeChange(const QString& obj_name, int mode) {
	left_transmit_mode_ = static_cast<Mode>(mode);
}

void TransmitSystem::ReceiveJointsAngle(const QString& obj_name, std::vector<float> joints) {		
	if (left_transmit_mode_ != eNormal) {
		emit SendPlanningDatas(obj_name, joints);
	}
	else {
		Model* robot = ModelManager::getInstance().GetModelByName(obj_map[obj_name].toStdString());
		if (!robot)		 return;
		auto   joints_sdk = robot->GetComponent<JointGroupComponent>(JointGroupComponent::type_name.data());
		if (!joints_sdk) return;
		joints_sdk->SetPositions(joints);
	}
}

void TransmitSystem::ProcessRobotTransmitDatas(const QString& obj_name, int type, std::vector<float> datas) {
	if (left_transmit_mode_ == eR2V) {
		switch (type) {
		case KUKA::DataType::JointCurPos: {
			Model* robot  = ModelManager::getInstance().GetModelByName(obj_map[obj_name].toStdString());
			if (!robot)  return;
			auto   joints_sdk = robot->GetComponent<JointGroupComponent>(JointGroupComponent::type_name.data());
			if (!joints_sdk) return;			
			joints_sdk->SetPositions(datas);
		}
		}
	}
}

} // !namespace GComponent