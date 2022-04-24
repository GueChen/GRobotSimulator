#ifndef ROBOT_BODY_MODEL_H
#define ROBOT_BODY_MODEL_H

#include <memory>
#include "model/model.h"

namespace GComponent {

class MyGL;
class Mesh;
class MyShader;

using std::unique_ptr;
using std::shared_ptr;

class ROBOT_BODY_MODEL: public Model
{
public:
    explicit ROBOT_BODY_MODEL(mat4 transform = mat4(1.0f));
    ~ROBOT_BODY_MODEL() = default;
    void Draw(MyShader * shader) override;

    static void setGL(const shared_ptr<MyGL>& other);
private:
    void InitializeResource();

    // FIXME:见KUKA_IIWA_MODEL 以及 10-27 思考整合 Model 部分
    static bool hasInit;
    static unique_ptr<Mesh> Resource;
};

}


#endif // ROBOT_BODY_MODEL_H
