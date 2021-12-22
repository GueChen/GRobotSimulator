#ifndef GMOTIONBASE_H
#define GMOTIONBASE_H
#include <vector>
#include <iostream>
#include <execution>
#include <algorithm>
#include <functional>

#include <GComponent/GTransform.hpp>
#include <GComponent/GGeometry.hpp>
#include <GComponent/GNumerical.hpp>

namespace GComponent {

using std::tuple;
using std::vector;
using std::function;
using Time = double;
using JointPosMsgFun = function<vector<double>(double)>;
using JointVelMsgFun = function<vector<double>(double)>;
using JointCruveMsgPkg = std::tuple<Time, JointPosMsgFun, JointVelMsgFun>;

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
