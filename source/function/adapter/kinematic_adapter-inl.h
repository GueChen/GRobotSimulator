/**
 *  @file  	kinematic_adpater-inl.h
 *  @brief 	The Implementation about Kinematic parameters Conversion Methods.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Jun 2nd, 2022
 **/
#ifndef _KINEMATIC_ADAPTER_INL_H
#define _KINEMATIC_ADAPTER_INL_H
#include "function/adapter/kinematic_adapter.h"
#include <iostream>

namespace GComponent {
template<class _Scaler>
LocalTransformsSE3<_Scaler> LocalTransformsSE3<_Scaler>::fromMDH(const vector<vector<_Scaler>>& mdh_params)
{
	assert(mdh_params[0].size() == 4);
	LocalTransformsSE3<_Scaler> SE3s;	
	vector<SE3<_Scaler>> & transform_matrices = SE3s.matrices_;
	transform_matrices.resize(mdh_params.size());
	for (int idx = 0; idx < mdh_params.size(); ++idx) 
	{
		const vector<_Scaler>& param = mdh_params[idx];
		transform_matrices[idx] = RobotKinematic::ModifiedDH(param[0], param[1], param[2], param[3]);
	}
	return SE3s;
}

template<class _Scaler>
LocalTransformsSE3<_Scaler> LocalTransformsSE3<_Scaler>::fromSDH(const vector<vector<_Scaler>>& sdh_params)
{
	assert(sdh_params[0].size() == 4);
	LocalTransformsSE3<_Scaler> SE3s;
	vector<SE3<_Scaler>>& transform_matrices = SE3s.matrices_;
	transform_matrices.resize(sdh_params.size());
	for (int idx = 0; idx < sdh_params.size(); ++idx)
	{
		const vector<_Scaler>& param = sdh_params[idx];
		transform_matrices[idx] = RobotKinematic::StandardDH(param[0], param[1], param[2], param[3]);
	}
	return SE3s;
}

template<class _Scaler>
pair<vector<Twist<_Scaler>>, SE3<_Scaler>> LocalTransformsSE3<_Scaler>::toTwists()
{
	vector<Twist<_Scaler>> twists(matrices_.size());
	SE3<_Scaler> transform_matrix;
	transform_matrix.setIdentity();
	for (int idx = 0; idx < matrices_.size(); ++idx) {
		transform_matrix = transform_matrix * matrices_[idx];
		std::cout << transform_matrix << std::endl;
		auto [R, p] = RtDecompositionMat4(transform_matrix);
		Vector<_Scaler, 3> w = R * Vector<_Scaler, 3>::UnitZ();
		twists[idx] = ScrewToTwist(p, w);	
	}
	return { twists, transform_matrix };
}


}

#endif // !_KINEMATIC_ADAPTERINL_H

