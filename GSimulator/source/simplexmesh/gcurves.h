#ifndef GCOMPONENT_GCURVES_H
#define GCOMPONENT_GCURVES_H

#include "simplexmesh/simplexmodel.hpp"

#include <glm/gtx/compatibility.hpp>
#include <GComponent/GGeometry.hpp>

#include <vector>

namespace GComponent {

using std::vector;

class GCurves : public GComponent::SimplexModel
{
/// 数据域 Fields
public:
    /* 顶点属性 */
    vector<ColorVertex> verteces;

/// 成员函数 Member Functions
public:
    GCurves(vector<vec3> poses,
            vec3 color1 = vec3(1.0f), vec3 color2 = vec3(1.0f));
    ~GCurves() = default;

    void Draw(MyShader * shader = nullptr);

private:
    /* GL 资源初始化函数 */
    void GLBufferInitialize() override;

};

} // namespace GComponent

#endif // GCOMPONENT_GCURVES_H
