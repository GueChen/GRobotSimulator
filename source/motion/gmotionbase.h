#ifndef GMOTIONBASE_H
#define GMOTIONBASE_H

#include <GComponent/GTransform.hpp>
#include <GComponent/GGeometry.hpp>
#include <GComponent/GNumerical.hpp>

#include <vector>
#include <string>
#include <map>
#include <unordered_map>

#include <execution>
#include <algorithm>
#include <functional>

#include <iostream>

namespace GComponent {

using std::function;

using std::string;
using std::tuple;
using std::map;
using std::vector;
using std::unordered_map;

using Time = double;
using JointPos = vector<double>;
using JointVel = vector<double>;
using JointPair = pair<JointPos, JointVel>;
using JointPosPair = pair<JointPos, JointPos>;

using DualJointsPosFun = function<JointPosPair(Time)>;
using JointPosMsgFun   = function<JointPos(Time)>;
using JointVelMsgFun   = function<JointVel(Time)>;
using JointPairMsgFun  = function<pair<JointPos, JointVel>(Time)>;
using JointCruveMsgPkg = tuple<Time, JointPosMsgFun, JointVelMsgFun>;
using DualJointMsgPkg  = tuple<Time, DualJointsPosFun>;

enum class InterpolationEnum{
    Simple,
    Quadratic,
    Cubic,
    Quintic,
    Trapezoid
};

class GMotionBase
{
public:
    GMotionBase();
};

}
#endif // GMOTIONBASE_H
