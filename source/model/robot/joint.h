#ifndef JOINT_H
#define JOINT_H

#include <memory>

namespace GComponent {

class Model;

using std::shared_ptr;
class Joint
{
public:
    explicit Joint(Model* model) :_model(model) {}
    virtual ~Joint() = 0{}
protected:
    Model * _model;
};

class Revolute:public Joint
{
public:
    Revolute() = delete;
    explicit Revolute(Model* model);
    ~Revolute() = default;
    void Rotate(float angle);
    float getAngle();
};

}


#endif // JOINT_H
