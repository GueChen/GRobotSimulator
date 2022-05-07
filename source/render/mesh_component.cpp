#include "component/mesh_component.h"

#include "render/mygl.hpp"

#include <QtGUI/QOpenGLExtraFunctions>

#ifdef _DEBUG
#include <iostream>
#include <format>
#endif // _DEBUG

using namespace GComponent;

MeshComponent::MeshComponent(std::vector<Vertex> vertices, std::vector<Triangle> indices, std::vector<Texture> textures)
{
    mesh_datas_.Vertices = vertices;
    mesh_datas_.Indices = indices;
    mesh_datas_.Textures = textures;
    VAO = 0;
}

MeshComponent::~MeshComponent()
{
    CheckClearGL();
}

void MeshComponent::setupMesh()
{
    if (!HaveSetup) {
        std::vector<Vertex>& Vertices = mesh_datas_.Vertices;
        std::vector<Triangle>& Indices = mesh_datas_.Indices;

        /* 检查 Vertex 数据是否有误/为空 */
        if (Vertices.empty()) return;

        /* 绑定 VAO、VBO 与 EBO */
        const size_t BufferSize = Vertices.size() * sizeof(Vertex);
        std::tie(VAO, VBO) = gl->genVABO(&Vertices[0], BufferSize);
        gl->glBindVertexArray(VAO);
        EBO = gl->genEBO(Indices);

        /* 激活顶点数据 */
        gl->EnableVertexAttribArrays(3, 3, 2);
        gl->glBindVertexArray(0);

        /* 标志位置true */
        HaveSetup = true;
    }
#ifdef _DEBUG
    std::cout << std::format("VAO:[{: >5}]| VBO:[{: >5}]| EBO:[{: >5}]| gl:{}\n", VAO, VBO, EBO, QOpenGLContext::currentContext()->defaultFramebufferObject());
#endif // _DEBUG

}

void MeshComponent::setGL(const std::shared_ptr<MyGL>& other)
{
    /* 为 GL 指针传递 Context */
    gl = other;

    /* 申请资源前检查是否需要清理 */
    CheckClearGL();

    /* 申请传递 Vertex 资源 */
    setupMesh();
}

void MeshComponent::CheckClearGL()
{
    if (HaveSetup) {
        gl->glDeleteBuffers(1, &VBO);
        gl->glDeleteBuffers(1, &EBO);
        gl->glDeleteVertexArrays(1, &VAO);

        VAO = VBO = EBO = 0;
        HaveSetup = false;
    }
}

void MeshComponent::Draw()
{
    gl->glBindVertexArray(VAO);
    gl->glDrawElements(GL_TRIANGLES, 3 * mesh_datas_.Indices.size(), GL_UNSIGNED_INT, 0);
    gl->glBindVertexArray(0);
}
