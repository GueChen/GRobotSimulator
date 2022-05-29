#include "gcurves.h"



#include <algorithm>
#include <execution>

namespace GComponent {

GCurves::GCurves(vector<vec3> poses, vec3 c1, vec3 c2)
{
    if(poses.size() < 2) throw("ERROR Size");

    verteces.resize(poses.size());
    double step = 1.0 / (poses.size() - 1);
    std::transform(std::execution::seq,
                   poses.begin(), poses.end(), verteces.begin(),
                   [c1, c2, t = 0.0, step](auto && input)mutable
    {
        ColorVertex vertex;

        vertex.Position = input;
        vertex.Color = glm::mix(c1, c2, t);
        t += step;
        return vertex;
    });

}

void GCurves::Draw(MyShader *shader)
{
    if(!isInit) return;
    gl->glBindVertexArray(VAO);
    gl->glDrawArrays(GL_LINE_STRIP, 0, verteces.size());

}

void GCurves::GLBufferInitialize()
{
    if(isInit) return;

    const size_t VertNum        = verteces.size();
    const size_t ColorVert_SIZE = GCONST::VEC3_SIZE * 3 + GCONST::VEC2_SIZE;

    /* 申请 GPU 内存区, 并填充内存 */
    std::tie(VAO, VBO) = gl->genVABO(&verteces[0], ColorVert_SIZE * VertNum);

    /* 激活顶点数据 */
    gl->EnableVertexAttribArrays(3, 3, 2, 3);

}

} // namespace GComponent
