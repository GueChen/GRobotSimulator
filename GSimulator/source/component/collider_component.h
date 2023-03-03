/**
 *  @file  	kinematic_component.h
 *  @brief 	This class is used for an abstract about physics collider.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Aug 8th, 2022
 **/
#ifndef __COLLIDER_COMPONENT_H
#define __COLLIDER_COMPONENT_H

#include "component/component.hpp"
#include "physics/abstract_shape.hpp"

#include <vector>
#include <memory>

namespace GComponent {

// TODO: finish this class
class ColliderComponent: public Component{
	using _ShapePtr		= std::unique_ptr<AbstractShape>;
	using _ShapePtrs    = std::vector<std::unique_ptr<AbstractShape>>;
	using _ShapeRawPtrs = std::vector<AbstractShape*>;
	using _ShapeRawPtr  = AbstractShape*;
	
public:
	explicit	  ColliderComponent(Model* parent = nullptr, _ShapeRawPtrs shapes= {});

	void		  RegisterShape(_ShapeRawPtr ptr);
	_ShapeRawPtrs GetShapes();

	virtual const string_view& GetTypeName() const override;

protected:
	_ShapePtrs shapes_ = {};

public:
	constexpr static const std::string_view type_name = "ColliderComponent";
};
} // !namespace GComponent

#endif // !__COLLIDER_COMPONENT_H
