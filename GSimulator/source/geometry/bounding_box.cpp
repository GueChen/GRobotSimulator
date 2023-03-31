#include "geometry/bounding_box.h"

namespace GComponent {
Vec3f BoundingBox::Differ() const
{
	return m_max - m_min;
}

Vec3f BoundingBox::Centroid() const
{
	return 0.5f * m_min + 0.5f * m_max;
}

BoundingBox::Dim BoundingBox::MaxExtent() const
{
	Vec3f differ = Differ();
	if (differ.x() >= differ.y() && differ.x() >= differ.z()) {
		return Dim::X;
	}
	else if (differ.y() >= differ.z()) {
		return Dim::Y;
	}
	else {
		return Dim::Z;
	}
	
}

float BoundingBox::SurfaceArea() const
{
	Vec3f differ = Differ();
	return 2.0f * (differ.x() * differ.y() +	// bottom surface
				   differ.x() * differ.z() +	// front
				   differ.y() * differ.z());	// right
}

Vec3f BoundingBox::RelativeScale(const Vec3f& p) const
{
	Vec3f rel = p - m_min;
	if (m_max.x() > m_min.x()) rel.x() /= m_max.x() - m_min.x();
	if (m_max.y() > m_min.y()) rel.y() /= m_max.y() - m_min.y();
	if (m_max.z() > m_min.z()) rel.z() /= m_max.z() - m_min.z();
	return rel;
}

BoundingBox& BoundingBox::Merge(const BoundingBox& other)
{
	*this = MergeTwoBoundingBox(*this, other);
	return *this;	
}

BoundingBox& BoundingBox::Merge(const Vec3f& p)
{
	*this = MergeWithPoint(*this, p);
	return *this;
}


/*_______________________________________________STATIC METHODS_____________________________________________*/
BoundingBox BoundingBox::MergeTwoBoundingBox(const BoundingBox& box_a, const BoundingBox& box_b)
{	
	return BoundingBox(
		Vec3f(std::min(box_a.m_min.x(), box_b.m_min.x()),
			  std::min(box_a.m_min.y(), box_b.m_min.y()),
			  std::min(box_a.m_min.z(), box_b.m_min.z())),
		Vec3f(std::max(box_a.m_max.x(), box_b.m_max.x()),
			  std::max(box_a.m_max.y(), box_b.m_max.y()),
			  std::max(box_a.m_max.z(), box_b.m_max.z()))
	);
}

BoundingBox BoundingBox::MergeWithPoint(const BoundingBox& box, const Vec3f& p)
{
	return BoundingBox(
		Vec3f(std::min(box.m_min.x(), p.x()),
			  std::min(box.m_min.y(), p.y()),
			  std::min(box.m_min.z(), p.z())),
		Vec3f(std::max(box.m_max.x(), p.x()),
			  std::max(box.m_max.y(), p.y()),
			  std::max(box.m_max.z(), p.z()))
	);
}

BoundingBox BoundingBox::CompouteBoundingBox(const AbstractShape& shape, const Vec3f& trans, const SO3f& rot)
{
	BoundingBox bound_box;
	switch (shape.GetShapeType()) {
	case ShapeEnum::Sphere: {
		const SphereShape& sphere	= dynamic_cast<const SphereShape&>(shape);
		const Vec3f extend = sphere.m_radius * Vec3f::Ones();
		bound_box.m_min = trans - extend;
		bound_box.m_max = trans + extend;
		break;
	}
	case ShapeEnum::Capsule: {
		const CapsuleShape& capsule = dynamic_cast<const CapsuleShape&>(shape);
		const Vec3f half   = capsule.m_half_height * (rot * Vec3f::UnitZ());
		const Vec3f extend(std::abs(half.x()) + capsule.m_radius,
						   std::abs(half.y()) + capsule.m_radius,
						   std::abs(half.z()) + capsule.m_radius);
		bound_box.m_min = trans - extend;
		bound_box.m_max = trans + extend;
		break;
	}
	case ShapeEnum::Box: {
		const BoxShape&		box		= dynamic_cast<const BoxShape&>(shape);
		Vec3f ori_extend(box.m_half_x, box.m_half_y, box.m_half_z);
		Vec3f extend = Vec3f::Zero();
		for (int j = 0; j < 3; ++j) for (int i = 0; i < 3; ++i){
			extend(i) += std::abs(rot(i, j)) * ori_extend(j);			
		}		
		extend.x() = std::abs(extend.x());
		extend.y() = std::abs(extend.y());
		extend.z() = std::abs(extend.z());
		bound_box.m_min = trans - extend;
		bound_box.m_max = trans + extend;
		break;
	}
	case ShapeEnum::ConvexHull: {
		const ConvexShape&	convex	= dynamic_cast<const ConvexShape&>(shape);
		for (auto& vert : convex.m_positions) {
			Vec3f cur_vert = rot * vert + trans;
			bound_box.m_min.x() = std::min(bound_box.m_min.x(), cur_vert.x());
			bound_box.m_min.y() = std::min(bound_box.m_min.y(), cur_vert.y());
			bound_box.m_min.z() = std::min(bound_box.m_min.z(), cur_vert.z());
			bound_box.m_max.x() = std::max(bound_box.m_max.x(), cur_vert.x());
			bound_box.m_max.y() = std::max(bound_box.m_max.y(), cur_vert.y());
			bound_box.m_max.z() = std::max(bound_box.m_max.z(), cur_vert.z());
		}
		break;
	}
	case ShapeEnum::Plane: {
		// TODO: add plane implementation
		assert(false && "No Implementation");
		break;
	}
	case ShapeEnum::None: {
		assert(false && "Use none shape, cant cast bounding box");
		break;
	}
	}
	return bound_box;
}


}