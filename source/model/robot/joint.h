#ifndef JOINT_H
#define JOINT_H

#include <memory>

namespace GComponent {

class Model;

using std::shared_ptr;
class Joint
{
public:
    explicit Joint(const shared_ptr<Model> & model){_model = model.get();}
    virtual ~Joint() = 0{}
protected:
    Model * _model;
};

class Revolute:public Joint
{
public:
    Revolute() = delete;
    explicit Revolute(const shared_ptr<Model> & model);
    ~Revolute() = default;
    void Rotate(float angle);
    float getAngle();
};

}


#endif // JOINT_H
