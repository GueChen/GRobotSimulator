#ifndef _TRANSFORM_H
#define _TRANSFORM_H

#include "Component/component.hpp"

#include "GComponent/gtransform.hpp"

#include <Eigen/Dense>

namespace GComponent {
using Vector3 = Eigen::Vector3f;

class Transform : public Component {		
public:
	Transform() = default;

	~Transform() = default;

	Vector3 getPosition () const { return position_; }
	Vector3 getScale	() const { return scale_; }
	Vector3 getRotation () const { return rotation_; }

	void setPosition(const Vector3 & position);
	void setScale	(const Vector3& scale);
	void setRotation(const Vector3& rotation);

	SE3f getMatrix4x4() const;

protected:
	Vector3 position_ = Vector3::Zero();
	Vector3 rotation_ = Vector3::Zero();
	Vector3 scale_    = Vector3::Ones();
};
}

#endif // !_TRANSFORM_H
