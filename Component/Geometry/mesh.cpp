#include "mesh.h"
#include "Component/GLTooler.hpp"
#include <QOpenGLExtraFunctions>

using namespace GComponent;

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned> indices, std::vector<Texture> textures)
{
    Vertices = vertices;
    Indices  = indices;
    Textures = textures;
}

void Mesh::setupMesh()
{
    std::tie(VAO, VBO) = gl->genVABO(&Vertices[0], Vertices.size() * sizeof(Vertex));

    gl->glBindVertexArray(VAO);
    EBO = gl->genEBO(Indices);
    gl->EnableVertexAttribArrays(3, 3, 2);
    gl->glBindVertexArray(0);
}

void Mesh::setGL(std::shared_ptr<MyGL> other)
{
    gl = other;
    setupMesh();
}

void Mesh::Draw()
{

}
