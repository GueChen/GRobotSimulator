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
    explicit ROBOT_BODY_MODEL(mat4 transform = mat4(1.0f));
    ~ROBOT_BODY_MODEL() = default;

/// Tick Functions
    void tickImpl(float delta_time) override;

    void Draw(MyShader * shader) override;
protected:
    void setShaderProperty(MyShader& shader) override;

private:
    void InitializeModelResource();

    vec3        color_ = vec3(0.85f, 0.85f, 0.75f);

    static bool is_init_;
    static size_t count_;
};

}


#endif // ROBOT_BODY_MODEL_H
