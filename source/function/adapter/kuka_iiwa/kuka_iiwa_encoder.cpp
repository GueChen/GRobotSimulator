#include "kuka_iiwa_encoder.h"

namespace GComponent::KUKA {

Encoder::Encoder():
	msg_(QByteArray(4, 0))
{}

Encoder& Encoder::EncodeMoveSmartServoMotion(const std::vector<float>& datas)
{
	return 	EncodeJointsGoal(datas).
			EncodeCommand	(MessageType::RobotMove, 
							 MoveType::MoveAsync, 
							 MotionType::SmartServo, 
							 1);
}

Encoder& Encoder::EncodeMovePTPMotion(const std::vector<float>& datas)
{
	return 	EncodeJointsGoal(datas).
			EncodeCommand	(MessageType::RobotMove, 
							 MoveType::Move, 
							 MotionType::PTP, 
							 1);
}

/*________________PROTECTED METHODS_______________________________________*/
Encoder& Encoder::EncodeCommand(MessageType msg_type, MoveType	move_type, MotionType motion_type, unsigned goal_num){
	EncodeMessageByte	(msg_type).
	EncodeMoveByte		(move_type).
	EncodeMotionByte	(motion_type).
	EncodeGoalNumByte	(goal_num);
	msg_.insert(0,			 "_CM");
	msg_.insert(msg_.size(), "_EE");
	msg_.push_back('#');
	return *this;
}

Encoder& Encoder::EncodeMessageByte(MessageType type){
	msg_[0] = static_cast<char>(type);
	return *this;
}

Encoder& Encoder::EncodeMoveByte(MoveType type) {
	msg_[1] = static_cast<char>(type);
	return *this;
}

Encoder& Encoder::EncodeMotionByte(MotionType type) {
	msg_[2] = static_cast<char>(type);
	return *this;
}

Encoder& Encoder::EncodeGoalNumByte(unsigned num) {
	msg_[3] = static_cast<char>(num);
	return *this;
}

Encoder& Encoder::EncodeJointsGoal(const std::vector<float>& jdatas) {
	assert(jdatas.size() == 7 && "KUKA IIWA has and only has 7 joints, encode failed on joints num.");
	msg_.push_back(static_cast<char>(GoalType::Joints));
	for (auto& jdata : jdatas) {
		short tmp = static_cast<short>(jdata* 10000.0f);		
		msg_.push_back(static_cast<char>((tmp >> 8) & 0xff));
		msg_.push_back(static_cast<char>( tmp       & 0xff));
	}
	return *this;
}

}
