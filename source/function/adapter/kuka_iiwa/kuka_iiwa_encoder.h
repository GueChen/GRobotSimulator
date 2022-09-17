#pragma once

#include <QtCore/QByteArray>

#include <vector>

namespace GComponent {

namespace KUKA {

enum class MessageType
{
	RobotMove = 0, RobotMode, RobotConf
};
enum class MoveType
{
	Move = 0, MoveAsync
};
enum class MotionType
{
	PTP = 0, CartesianPTP, Line, Circle, Spline, SmartServo, SmartServoLin
};
enum class GoalType
{
	Joints = 0, Frame, FrameRedundancy, JointsPair, FrameWithVelocity, FrameRedundancyWithVelocity
};

class Encoder {
public:
	Encoder();
	~Encoder()	= default;

	Encoder &				 EncodeMoveSmartServoMotion(const std::vector<float>& datas);
	Encoder &				 EncodeMovePTPMotion	   (const std::vector<float>& datas);	
	inline QByteArray		 GetCommand()				{ return msg_; };
		
protected:		
	Encoder &				 EncodeCommand	  (MessageType  msg_type, 
											   MoveType		move_type, 
											   MotionType	type, 
											   unsigned		num);
	Encoder &				 EncodeMessageByte(MessageType type);
	Encoder &				 EncodeMoveByte	  (MoveType type);
	Encoder &				 EncodeMotionByte (MotionType type);
	Encoder &				 EncodeGoalNumByte(unsigned num);
	Encoder &				 EncodeJointsGoal (const std::vector<float>& jdatas);

private:
	QByteArray msg_;

public:
	static inline QByteArray RequestController() { return QByteArray(kControlRequest) + kEndSymbol; }
	static inline QByteArray RequestQuit()		 { return QByteArray(kQuitRequest)    + kEndSymbol; }
	static inline QByteArray RequestAsync()		 { return QByteArray(kAsyncRequest)	  + kEndSymbol; }
	static inline QByteArray RequestAsyncStop()  { return QByteArray(kAsyncStopRequest) + kEndSymbol; }
	static inline QByteArray RequestCancel()	 { return QByteArray(kCancelRequest) + kEndSymbol; }

private:
	constexpr static 
		const char kControlRequest[]	  = "REQUEST";
	constexpr static 
		const char kQuitRequest[]		  = "quit";
	constexpr static 
		const char kAsyncRequest[]		  = "GETASYNC";
	constexpr static 
		const char kAsyncStopRequest[]    = "GETSTOP";
	constexpr static
		const char kCancelRequest[]		  = "CLEANUP";
	constexpr static 
		const char kEndSymbol			  = '#';
};

} // !namespace KUKA

} // !namespace GComponent