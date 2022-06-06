#ifndef ROBOT_BODY_MODEL_H
#define ROBOT_BODY_MODEL_H

#include <memory>
#include "model/model.h"

namespace GComponent {

class MyGL;
class RenderMesh;
class MyShader;

using std::unique_ptr;
using std::shared_ptr;

class ROBOT_BODY_MODEL: public Model
{
public:
    explicit ROBOT_BODY_MODEL(Mat4 transform = Mat4::Identity());
    ~ROBOT_BODY_MODEL() = default;

/// Tick Functions
    void tickImpl(float delta_time) override;

    void Draw(MyShader * shader) override;
protected:
    void setShaderProperty(MyShader& shader) override;

private:
    void InitializeModelResource();

    Vec3        color_ = Vec3(0.85f, 0.85f, 0.75f);

    static bool is_init_;
    static size_t count_;
};

}


#endif // ROBOT_BODY_MODEL_H
