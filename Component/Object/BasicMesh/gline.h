#ifndef GCOMPONENT_GLINE_H
#define GCOMPONENT_GLINE_H

#include "Component/GStruct.hpp"
#include "Component/Object/BasicMesh/SimplexModel.hpp"
#include "Component/Geometry/Vertex.hpp"

namespace GComponent {

class MyGL;
class MyShader;

class GLine : public GComponent::SimplexModel
{
public:
    /* 顶点属性项 */
    ColorVertex _vertBegin, _vertEnd;

private:
    constexpr static Line line{0 , 1};

public:
    GLine(vec3 pos1, vec3 pos2,
          vec3 color1 = vec3(1.0f), vec3 color2 = vec3(1.0f));
    ~GLine() = default;

    void Draw(MyShader* shader = nullptr) override;

private:
    /* GL 资源初始化函数 */
    void GLBufferInitialize() override;

};

} // namespace GComponent

#endif // GCOMPONENT_GLINE_H
