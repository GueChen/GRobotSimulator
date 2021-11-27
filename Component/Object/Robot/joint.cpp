#include "joint.h"
#include "Component/Object/model.h"

using namespace GComponent;

Revolute::Revolute(const shared_ptr<Model> & model):
    Joint(model)
{}


void Revolute::Rotate(float angle)
{
    _model->setRotate(angle);
}

