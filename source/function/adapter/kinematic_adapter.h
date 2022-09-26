/**
 *  @file  	kinematic_adpater.h
 *  @brief 	Some datas reforms for kinematic methods.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Jun 2nd, 2022
 **/
#ifndef _KINEMATIC_ADAPTER_H
#define _KINEMATIC_ADAPTER_H

#include <GComponent/grobotkinematic.h>
#include <GComponent/GTransform.hpp>

#include <Eigen/Dense>

#include <vector>

namespace GComponent{
using std::pair;
using std::vector;
template<class T>
using Twist  = Eigen::Vector<T, 6>;
using Twistd = Eigen::Vector<double, 6>;
using Twistf = Eigen::Vector<float, 6>;

template<class _Scaler>
class LocalTransformsSE3 {

public:
	
	~LocalTransformsSE3() = default;

	static LocalTransformsSE3 fromMDH(const vector<vector<_Scaler>>& mdh_params);
	static LocalTransformsSE3 fromSDH(const vector<vector<_Scaler>>& sdh_params);

	pair<vector<Twist<_Scaler>>, SE3<_Scaler>> toTwists();
	
	// TODO: add the methods to fit transform
	vector<vector<_Scaler>> toMDH(const vector<SE3<_Scaler>>& matrices);
	vector<vector<_Scaler>> toSDH(const vector<SE3<_Scaler>>& matrices);

	inline const vector<SE3<_Scaler>>& getMatrices() const { return matrices_; }
	inline const size_t size() const { return matrices_.size(); }
protected:
	LocalTransformsSE3() = default;

private:
	vector<SE3<_Scaler>> matrices_;
};

}

#include "function/adapter/kinematic_adapter-inl.h"

#endif // !_KINEMATC_ADPATER_H
