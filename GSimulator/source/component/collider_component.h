/**
 *  @file  	kinematic_component.h
 *  @brief 	This class is used for an abstract about physics collider.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Aug 8th, 2022
 **/
#ifndef __COLLIDER_COMPONENT_H
#define __COLLIDER_COMPONENT_H

#include "component/component.hpp"
#include "geometry/abstract_shape.hpp"
#include "geometry/bounding_box.h"

#include <vector>
#include <memory>

namespace GComponent {

// TODO: finish this class
class ColliderComponent: public Component{
	using _ShapePtr		= std::unique_ptr<AbstractShape>;
	using _ShapePtrs    = std::vector<std::unique_ptr<AbstractShape>>;
	using _Boundings    = std::vector<BoundingBox>;
	using _ShapeRawPtrs = std::vector<AbstractShape*>;
	using _ShapeRawPtr  = AbstractShape*;
	
public:
	explicit	  ColliderComponent(Model* parent = nullptr, _ShapeRawPtrs shapes= {});

	void		  RegisterShape	 (_ShapeRawPtr ptr);
	void		  DeregisterShape(_ShapeRawPtr ptr);

	_ShapeRawPtr  GetShape		 (int idx);
	_ShapeRawPtrs GetShapes		 ();

	inline bool	  IsStatic() const					{ return is_static_; }
	inline void	  SetStatic(bool flag)				{ is_static_ = flag; }

	inline const BoundingBox& GetBound() const		{ return bound_; }
	inline const _Boundings& GetShapesBounds()const { return boundings_; }

	virtual const string_view& GetTypeName() const override;

protected:
	void		  tickImpl(float delta) override;
	void		  UpdateBoundingBox(const BoundingBox& box);

	QJsonObject	  Save()						   override;
	bool		  Load(const QJsonObject& com_obj) override;

protected:	
	_ShapePtrs  shapes_	   = {};
	// TODO: create a new class combine shape and bounding box
	_Boundings  boundings_ = {};
	BoundingBox bound_;
	bool		is_static_  = false;

public:
	constexpr static const std::string_view type_name = "ColliderComponent";
};
} // !namespace GComponent

#endif // !__COLLIDER_COMPONENT_H
