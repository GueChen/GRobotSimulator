#ifndef GCOMPONENT_GBALL_H
#define GCOMPONENT_GBALL_H

#include "component/mesh_component.h"

#include "simplexmesh/simplexmodel.hpp"

namespace GComponent {

class GBall : public GComponent::SimplexModel
{
/// 数据域 Fields
public:
    vec3    center;
    float   radius;

private:
    MeshComponent    mesh;

/// 成员函数 Member Functions
public:
    GBall() = delete;
    GBall(vec3 o, float r, int resolution = 36);
    ~GBall() = default;

    /* 删除拷贝函数 */
    GBall(const GBall &) = delete;
    GBall & operator=(const GBall &) = delete;

    /* 保留移动函数 */
    GBall(GBall && other);
    GBall &
    operator=(GBall && other);

    /* 绘图函数重写 */
    void
    Draw(MyShader * shader) override;

    /* 移动控制接口函数 */
    void
    MoveBall(const vec3& v)
    {
        center += v;
    }

    /* 计算是否碰撞 */
    bool
    CalcCollision(vec3& p) const
    {
        return length(p - center) - 0.01f < radius;
    }

    /* 计算与球心的方向 */
    vec3
    GetOutDirection(vec3& p) const
    {
        return  center + (radius + 0.01f) * (p - center)/length(p - center);
    }

private:
    void GLBufferInitialize() override;

/// 辅助函数
    /* 填充网格 */
    void
    setupMesh(int lo, int la);
    /* 填充顶点 */
    std::vector<Vertex>
    setupVertex(int la, int lo);
    /* 填充三角面映射 */
    std::vector<Triangle>
    setupIndicies(int lo, int la);

};

} // namespace GComponent

#endif // GCOMPONENT_GBALL_H
