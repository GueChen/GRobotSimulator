/**
 *  @file  	kinematic_component.h
 *  @brief 	This class is used for an abstract about physics collider.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Aug 8th, 2022
 **/
#ifndef __COLLIDER_COMPONENT_H
#define __COLLIDER_COMPONENT_H

#include "component/component.hpp"

namespace GComponent {

// TODO: finish this class
class ColliderComponent:public Component{
public:
	virtual const string_view& GetTypeName() const override;

public:
	constexpr static const std::string_view type_name = "ColliderComponent";
};
} // !namespace GComponent

#endif // !__COLLIDER_COMPONENT_H
