/**
 *  @file  	tracker_component.h
 *  @brief 	This class Applies Bind to a Goal and Try use Kinematic Component to Closer Goal.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	May 25, 2022
 **/
#ifndef _TRACKER_COMPONENT_H
#define _TRACKER_COMPONENT_H

#include "component/component.hpp"

#include <functional>
#include <string>

namespace GComponent {
using std::string_view;
class TrackerComponent: public Component {

public:
	explicit	TrackerComponent(Model* ptr_parent, Model* goal) : Component(ptr_parent), tracked_goal_(goal) {}
	virtual		~TrackerComponent() = default;

	virtual void tick(float delta_time) override;
	inline  void SetGoal(Model* goal) { tracked_goal_ = goal; }
	// 通过 Component 继承
	virtual const string_view& GetTypeName() const override { return type_name; }

private:
	Model*	tracked_goal_ = nullptr;
	static constexpr const string_view type_name = "TrackerComponent";
};
}

#endif // !_TRACKER_COMPONENT_H

