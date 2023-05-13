#include "component/tracker_component.h"

// TODO: Replace it with Kinematic Component
#include "manager/modelmanager.h"
#include "component/kinematic_component.h"
#include "component/joint_group_component.h"
#include "component/transform_component.h"

#ifdef _DEBUG
#include <iostream>
#include <format>
#endif // _DEBUG

namespace GComponent{

unordered_set<string> TrackerComponent::trackable_table_ = {};

TrackerComponent::TrackerComponent(Model* ptr_parent, const string& goal_name) :
	Component(ptr_parent)
{
	trackable_table_.insert(GetName());
	SetGoal(goal_name);
}

TrackerComponent::~TrackerComponent()
{
	/* if there is a goal, make sure cleaning possession of self */
	if (tracked_goal_) {
		_RawPtr goal_component = tracked_goal_->GetComponent<_Self>();
		if (goal_component) {
			goal_component->DeregisterTracer(GetName());
		}
	}
	/* if is tracked, cleaning the possesion of all tracers */
	ClearTracers();
	trackable_table_.erase(GetName());
}

void TrackerComponent::SetGoal(const string& goal_name)
{
	if (goal_name.empty() || goal_name == "NULL") {
		tracked_goal_ = nullptr;
		return;
	}

	if (!trackable_table_.count(goal_name)) return;
	tracked_goal_ = ModelManager::getInstance().GetModelByName(goal_name);
	if (tracked_goal_) {
		_RawPtr goal_component = tracked_goal_->GetComponent<_Self>();
		if (goal_component) {
			goal_component->RegisterTracer(GetName());
		}
	}
}

void TrackerComponent::RegisterTracer(const string& tracer_name)
{
	auto it = tracer_node_map_.find(tracer_name);
	if (tracer_node_map_.end() != it) return;
	tracer_list_.push_front(tracer_name);
	tracer_node_map_.emplace(tracer_name, tracer_list_.begin());
}

void TrackerComponent::DeregisterTracer(const string& tracer_name)
{
	auto it = tracer_node_map_.find(tracer_name);
	if (tracer_node_map_.end() == it) return;
	tracer_list_.erase(it->second);
	tracer_node_map_.erase(it);
}

void TrackerComponent::ClearTracers()
{
	for (auto& tracer_name : tracer_list_) {
		Model* tracer_ptr = ModelManager::getInstance().GetModelByName(tracer_name);
		if (tracer_ptr) {
			_RawPtr tracer_component = tracer_ptr->GetComponent<_Self>();
			if (tracer_component) {
				tracer_component->SetGoal("");
			}
		}
	}
}

void GComponent::TrackerComponent::tickImpl(float delta_time)
{
	if (state_ == State::Sleeping) return;
	static int count = 0;
	Model* ptr_parent = GetParent();
	if (!tracked_goal_ || !ptr_parent) return;
	TransformCom* trans = ptr_parent->GetTransform();
	TransformCom* goal  = tracked_goal_->GetTransform();
	Mat4 goal_local_SE3  = InverseSE3(trans->GetModelGlobal()) * goal->GetModelMatrixWithoutScale();

	KinematicComponent*  kinematic_component   = ptr_parent->GetComponent<KinematicComponent>();
	
	Thetas<float> out_thetas;
	if (kinematic_component) {
		JointGroupComponent* joint_group_component = ptr_parent->GetComponent<JointGroupComponent>();		 		
		bool result  = kinematic_component->InverseKinematic(out_thetas, goal_local_SE3, joint_group_component->GetPositions());
		std::ranges::transform(out_thetas, out_thetas.begin(), ToStandarAngle);
		if (result) {
			joint_group_component->SetPositions(out_thetas);				
		}

	}
}

QJsonObject TrackerComponent::Save()
{
	QJsonObject com_obj;
	com_obj["type"]			= type_name.data();
	com_obj["state"]        = static_cast<int>(state_);
	com_obj["tracked_goal"] = QString::fromStdString(tracked_goal_ ? tracked_goal_->getName() : "null");
	QJsonArray traces_obj;
	for (auto& tracer : tracer_list_) {
		traces_obj.append(QString::fromStdString(tracer.data()));
	}

	return com_obj;
}

}