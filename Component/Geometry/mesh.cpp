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
    if(VAO != 0)
    {
        gl->glDeleteVertexArrays(1, &VAO);
        gl->glDeleteBuffers(1, &VBO);
        gl->glDeleteBuffers(1, &EBO);
    }
}


void Mesh::setupMesh()
{
    std::tie(VAO, VBO) = gl->genVABO(&Vertices[0], Vertices.size() * sizeof(Vertex));

    gl->glBindVertexArray(VAO);
    EBO = gl->genEBO(Indices);
    gl->EnableVertexAttribArrays(3, 3, 2);
    gl->glBindVertexArray(0);
    qDebug() << "VAO:" << VAO;

}

void Mesh::setGL(std::shared_ptr<MyGL> other)
{
    gl = other;

    setupMesh();
    HaveSetup = true;

}

void Mesh::Draw()
{
    gl->glBindVertexArray(VAO);
    gl->glDrawElements(GL_TRIANGLES, Indices.size(), GL_UNSIGNED_INT, 0);
    gl->glBindVertexArray(0);
}
