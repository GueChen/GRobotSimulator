#pragma once

#include <string>
#include <vector>


namespace GComponent {

namespace KUKA {

union ShortUnion
{
	short data;
	char  bytes[2];
};

enum DataType : uint8_t{
	JointCurPos			= 0, 
	JointCommandPos		= 1,
	CartCurPos			= 2,
	CartCommandPos		= 3,
	Wrench				= 4,
	ExtJTorque			= 5,
	RawJTorque			= 6,
	Msg					= 200,
	None				= 255
};

class Decoder
{
public:
	Decoder(char* data, size_t size);
	~Decoder() = default;

	std::vector<float> GetDatas();
	DataType		   GetType();
	int				   GetRankInfo();	
	bool			   GetAsyncInfo();
	bool			   IsEffective();
	bool			   IsMessageType();
	bool			   IsAsyncStatus();
protected:
	std::vector<float> DecodeAsJoints();
	std::vector<float> DecodeAsCartesian();
	std::vector<float> DecodeAsWrench();
	std::vector<float> DecodeAsJTorque();
	
	short			   ProcessBytes(char* data);

	

private:
	char*	 data_	= nullptr;
	size_t	 size_	= 0;	
	static constexpr const uint32_t kBegin = 4u;
};

} // !namespace KUKA

} // !namespace GComponent
