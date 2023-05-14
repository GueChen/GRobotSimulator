#ifndef __AUBO_I3_ROBOT_H
#define __AUBO_I3_ROBOT_H

#include "GComponent/gtransform.hpp"

#include "model/model.h"

#include <Eigen/Dense>

namespace GComponent {

class AUBO_I3_MODEL : public Model {
public:
	explicit AUBO_I3_MODEL(Model* parent_ptr, Mat4 transform = Mat4::Identity());
	~AUBO_I3_MODEL() = default;

	void tickImpl(float delta_time) override;

private:
	void InitializeModelResource();
	void InitializeMeshResource ();	
};

}

#endif // !__AUBO_I3_ROBOT_H
