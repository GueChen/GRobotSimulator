#include "mesh.h"
#include "Component/MyGL.hpp"
#include <QOpenGLExtraFunctions>

using namespace GComponent;

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned> indices, std::vector<Texture> textures)
{
    Vertices = vertices;
    Indices  = indices;
    Textures = textures;
    VAO = 0;
}

// TODO: 需要添加一个引用计数
Mesh::~Mesh()
{
    CheckClearGL();
}


void Mesh::setupMesh()
{
    if(!HaveSetup) {
    // TODO: 可以加入一个断言或异常
    /* 检查 Vertex 数据是否有误/为空 */
    if(Vertices.empty()) return;

    /* 绑定 VAO、VBO 与 EBO */
    const size_t BufferSize = Vertices.size() * sizeof (Vertex);
    std::tie(VAO, VBO) = gl->genVABO(&Vertices[0], BufferSize);
    gl->glBindVertexArray(VAO);
    EBO = gl->genEBO(Indices);

    /* 激活顶点数据 */
    gl->EnableVertexAttribArrays(3, 3, 2);
    gl->glBindVertexArray(0);

    /* 标志位置true */
    HaveSetup = true;
    }
}

void Mesh::setGL(const std::shared_ptr<MyGL> & other)
{
    /* 为 GL 指针传递 Context */
    gl = other;

    /* 申请资源前检查是否需要清理 */
    CheckClearGL();

    /* 申请传递 Vertex 资源 */
    setupMesh();
}

void Mesh::CheckClearGL()
{
    if(HaveSetup)  {
    gl->glDeleteBuffers(1, &VBO);
    gl->glDeleteBuffers(1, &EBO);
    gl->glDeleteVertexArrays(1, &VAO);

    VAO = VBO = EBO = 0;
    HaveSetup = false;
    }
}

void Mesh::Draw()
{
    gl->glBindVertexArray(VAO);
    gl->glDrawElements(GL_TRIANGLES, Indices.size(), GL_UNSIGNED_INT, 0);
    gl->glBindVertexArray(0);
}
