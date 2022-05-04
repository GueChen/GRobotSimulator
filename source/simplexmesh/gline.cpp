#include "gline.h"

#include "render/mygl.hpp"

#include <iostream>

namespace GComponent {

GLine::GLine(vec3 p1, vec3 p2, vec3 c1, vec3 c2)
{
    vert_begin.Position = p1;
    vert_begin.Color    = c1;

    vert_end.Position   = p2;
    vert_end.Color      = c2;
    std::cout << p1.x << ", " << p1.y << ", " << p1.z << std::endl;
}

void GLine::Draw(MyShader * shader)
{
    // TODO: 考虑抛出异常提示初始化？
    if(!isInit) return;

    gl->glBindVertexArray(VAO);
    gl->glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);
}

void GLine::GLBufferInitialize()
{
    if(isInit) return;

    const size_t VertNum        = 2;
    const size_t ColorVert_SIZE = GCONST::VEC3_SIZE * 3 + GCONST::VEC2_SIZE;

    /* 申请 GPU 内存区 */
    std::tie(VAO, VBO) = gl->genVABO(nullptr, ColorVert_SIZE * VertNum);
    std::cout << "GLines:\n"
                 "VAO:= " << VAO << ", VBO:= " << VBO << std::endl;

    /* 填充数据 */
    gl->glBindVertexArray(VAO);
    gl->glBufferSubData(GL_ARRAY_BUFFER,              0, ColorVert_SIZE, &vert_end);
    gl->glBufferSubData(GL_ARRAY_BUFFER, ColorVert_SIZE, ColorVert_SIZE, &vert_begin);

    /* 生成元素映射 */
    EBO = gl->genEBO(vector{line});
    std::cout << "EBO:= " << EBO << std::endl;
    /* 激活顶点数据 */
    gl->EnableVertexAttribArrays(3, 3, 2, 3);
}

} // namespace GComponent
