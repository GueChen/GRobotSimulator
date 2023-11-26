#ifndef __AUBO_I5_ROBOT_H
#define __AUBO_I5_ROBOT_H

#include "GComponent/gtransform.hpp"

#include "model/model.h"

#include <Eigen/Dense>

namespace GComponent {

	class AUBO_I5_MODEL : public Model {
	public:
		explicit AUBO_I5_MODEL(Model* parent_ptr, Mat4 transform = Mat4::Identity());
		~AUBO_I5_MODEL() = default;

	private:
		void InitializeModelResource();
		void InitializeMeshResource();
	};



}

#endif // !__AUBO_I3_ROBOT_H