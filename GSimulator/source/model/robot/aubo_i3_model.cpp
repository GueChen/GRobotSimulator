#include "model/robot/aubo_i3_model.h"

#include "model/robot/robot_asset.h"
#include "model/robot/robot_model_builder.h"

#include "component/transform_component.h"

#include <utility>

namespace GComponent {

static bool aubo_resouce_init = false;

AUBO_I3_MODEL::AUBO_I3_MODEL(Model* parent_ptr, Mat4 transform):
	RobotModel(parent_ptr)
{
	GetTransform()->SetModelLocal(transform);
	InitializeMeshResource();
	InitializeModelResource();
}

void AUBO_I3_MODEL::tickImpl(float delta_time)
{
	
}

void AUBO_I3_MODEL::InitializeModelResource()
{
	auto built_model = RobotModelBuilder::Build(*this, GetAuboI3Asset());
	SetRobotRuntimeData(std::move(built_model.links), std::move(built_model.joints));
}

void AUBO_I3_MODEL::InitializeMeshResource()
{
	if (!aubo_resouce_init) {
		RobotModelBuilder::RegisterMeshes(GetAuboI3Asset());
		aubo_resouce_init = true;
	}
}

}