#include "component/transform.h"
namespace GComponent{

void Transform::setPosition(const Vector3 & position)
{
	position_ = position;
}

void Transform::setScale(const Vector3 & scale)
{
	scale_ = scale;
}

void Transform::setRotation(const Vector3 & rotation)
{
	rotation_ = rotation;
}

SE3d Transform::getMatrix4x4() const
{
	Eigen::Transform<double, 3, Eigen::Affine> transform;
	transform.setIdentity();
	transform.translate(position_);
	if(double angle = rotation_.norm(); abs(angle) > 1e-5)
	transform.rotate(Eigen::AngleAxisd(angle, rotation_ / angle));
	transform.scale(scale_);
	return transform.matrix();
}
}