#include "render/rendermesh.h"

#include "render/mygl.hpp"

#include <QtGUI/QOpenGLExtraFunctions>

#ifdef _DEBUG
#include <iostream>
#include <format>
#endif // _DEBUG

using namespace GComponent;

RenderMesh::RenderMesh(std::vector<Vertex> vertices, std::vector<Triangle> indices, std::vector<Texture> textures)
{
    mesh_datas_.Vertices = vertices;
    mesh_datas_.Indices = indices;
    mesh_datas_.Textures = textures;
    VAO_ = 0;
}

RenderMesh::~RenderMesh()
{
    CheckClearGL();
}

void RenderMesh::setupMesh()
{
    if (!is_setup_) {
        std::vector<Vertex>& Vertices = mesh_datas_.Vertices;
        std::vector<Triangle>& Indices = mesh_datas_.Indices;

        /* 检查 Vertex 数据是否有误/为空 */
        if (Vertices.empty()) return;

        /* 绑定 VAO、VBO 与 EBO */
        const size_t BufferSize = Vertices.size() * sizeof(Vertex);
        std::tie(VAO_, VBO_) = gl->genVABO(&Vertices[0], BufferSize);
        gl->glBindVertexArray(VAO_);
        EBO_ = gl->genEBO(Indices);

        /* 激活顶点数据 */
        gl->EnableVertexAttribArrays(3, 3, 2);
        gl->glBindVertexArray(0);

        /* 标志位置true */
        is_setup_ = true;
    }
#ifdef _DEBUG
    std::cout << std::format("VAO:[{: >5}]| VBO:[{: >5}]| EBO:[{: >5}]| gl:{}\n", VAO_, VBO_, EBO_, QOpenGLContext::currentContext()->defaultFramebufferObject());
#endif // _DEBUG

}

void RenderMesh::setGL(const std::shared_ptr<MyGL>& other)
{
    /* 为 GL 指针传递 Context */
    gl = other;

    /* 申请资源前检查是否需要清理 */
    CheckClearGL();

    /* 申请传递 Vertex 资源 */
    setupMesh();
}

void RenderMesh::CheckClearGL()
{
    if (is_setup_) {
        gl->glDeleteBuffers(1, &VBO_);
        gl->glDeleteBuffers(1, &EBO_);
        gl->glDeleteVertexArrays(1, &VAO_);

        VAO_ = VBO_ = EBO_ = 0;
        is_setup_ = false;
    }
}

void RenderMesh::Draw()
{
    gl->glBindVertexArray(VAO_);
    gl->glDrawElements(GL_TRIANGLES, 3 * mesh_datas_.Indices.size(), GL_UNSIGNED_INT, 0);
    gl->glBindVertexArray(0);
}
