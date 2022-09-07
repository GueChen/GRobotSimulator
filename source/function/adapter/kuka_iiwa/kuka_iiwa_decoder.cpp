#include "kuka_iiwa_decoder.h"

#include <unordered_map>

namespace GComponent::KUKA{

static std::unordered_map<std::string, int> rank_map = {
    {"MSG:LINK_SUCESS",             1},
    {"MSG:NOT_OBSERVER_ANYMORE",    2},
    {"MSG:WAIT_FOR_CONTROLLER",     2},
    {"MSG:GET_HIGHEST_AURTHORITY",  3}
    
};

bool CompareWithIdentifier(const char* source, const char* target, size_t size);

Decoder::Decoder(char* data, size_t size):
    data_(data), size_(size)
{}

std::vector<float> Decoder::GetDatas()
{
    if (!IsEffective()) return std::vector<float>{};
    DataType type = GetType();  
    switch (type) {
    case Wrench:            return DecodeAsWrench();         
    case JointCurPos: 
    case JointCommandPos:   return DecodeAsJoints();
    case CartCurPos:
    case CartCommandPos:    return DecodeAsCartesian();         
    case ExtJTorque:
    case RawJTorque:        return DecodeAsJTorque();         
    }    
}

DataType Decoder::GetType()
{    
    if (!IsEffective())  return DataType::None;
    return static_cast<DataType>(data_[3]);
}

int Decoder::GetRankInfo()
{
    return rank_map[std::string(data_, size_)];
}

bool Decoder::GetAsyncInfo()
{
    static const char kIdentifier[] = "ON";
    return CompareWithIdentifier(data_ + 6, kIdentifier, 2);
}

bool Decoder::IsEffective()
{
    static const char   kBeginIdentifier[] = "_ST";
    static const size_t kBeginSize         = sizeof kBeginIdentifier - 1;
    static const char   kEndIdentifier[]   = "_EE";
    static const size_t kEndSize           = sizeof kEndIdentifier - 1;
    return  size_ >= (kBeginSize + kEndSize) &&
        CompareWithIdentifier(data_, kBeginIdentifier, kBeginSize) &&
        CompareWithIdentifier(data_ + size_ - 3, kEndIdentifier, kEndSize);
}

bool Decoder::IsMessageType()
{
    static const char   kIdentifier[] = "MSG:";
    static const size_t kSize = sizeof kIdentifier - 1;
    return size_>= kSize && CompareWithIdentifier(data_, kIdentifier, kSize);
}

bool Decoder::IsAsyncStatus()
{
    static const char   kIdentifier[] = "ASYNC:";
    static const size_t kSize = sizeof kIdentifier - 1;    
    return size_ >= kSize && CompareWithIdentifier(data_, kIdentifier, kSize);
}

/*____________________________PROTECTED METHODS________________________________*/
std::vector<float> Decoder::DecodeAsJoints()
{
    std::vector<float> joints(7);
    for (int i = 0; i < 7; ++i) {                       
        joints[i] = ProcessBytes(&data_[kBegin + 2 * i]) * 0.0001f;
    }
    return joints;
}

std::vector<float> Decoder::DecodeAsCartesian()
{
    std::vector<float> cartpose(6);
    for (int i = 0; i < 6; ++i) {        
        cartpose[i] = ProcessBytes(&data_[kBegin + 2 * i]) * (i < 3 ? 0.1f : 0.0001f);
    }
    return cartpose;
}

std::vector<float> Decoder::DecodeAsWrench()
{
    std::vector<float> wrench(6);
    for (int i = 0; i < 6; ++i) {        
        wrench[i] = ProcessBytes(&data_[kBegin + 2 * i]) * 0.01f;
    }
    return wrench;
}

std::vector<float> Decoder::DecodeAsJTorque()
{
    std::vector<float> jtorque(7);
    for (int i = 0; i < 7; ++i) {        
        jtorque[i] = ProcessBytes(&data_[kBegin + 2 * i]) * 0.01f;
    }
    return jtorque;
}

short Decoder::ProcessBytes(char* data)
{    
    short tmp = 0;
    tmp |= (data[0] << 8);
    tmp |= (data[1] & 0xff);
    return tmp;
}

/*_______________Helper Function____________________________________________*/
bool CompareWithIdentifier(const char* source, const char* target, size_t size) {
    for (int i = 0; i < size; ++i) {
        if (source[i] != target[i]) return false;
    }
    return true;
}
}