#include "Gball.h"
#include <iostream>
#include "Component/myshader.h"

namespace GComponent {

GBall::GBall(vec3 o, float r, int resolution):
    center(o), radius(r), mesh({},{},{})
{
    setupMesh(resolution, 2 * resolution);

}

GBall::GBall(GBall && other):
    mesh({},{},{})
{
    VAO = other.VAO;
    VBO = other.VBO;
    EBO = other.EBO;
    gl = other.gl;
    isInit = other.isInit;

    std::swap(mesh, other.mesh);
    center = other.center;
    radius = other.radius;
}

GBall & GBall::operator=(GBall && other)
{
    VAO = other.VAO;
    VBO = other.VBO;
    EBO = other.EBO;
    gl = other.gl;
    isInit = other.isInit;

    std::swap(mesh, other.mesh);
    center = other.center;
    radius = other.radius;

    other.isInit = false;

    return *this;
}

void GBall::GLBufferInitialize()
{
    mesh.setGL(gl);
}

void GBall::Draw(MyShader *shader)
{
    if(!isInit) return;
    mat4 model = glm::identity<mat4>();
    model = glm::translate(model, center);
    shader->setMat4("model", model);
    mesh.Draw();
}

void GBall::setupMesh(int la, int lo)
{
    setupVertex(la, lo);
    setupIndicies(la, lo);
}

void GBall::setupVertex(int la, int lo)
{
    const float dLatitude   = glm::pi<float>() / la;
    const float dLongitude  = 2.0* glm::pi<float>() / lo;
    float deltaV = 1.0f / la;
    float deltaU = 1.0f / (lo - 1);

    for(int i = 0; i <= la; ++i)
    {
        /* 计算经度角 */
        float theta = -glm::pi<float>() + dLatitude * i;
        float sTh = sin(theta);
        float cTh = cos(theta);

        /* 计算贴图 V 坐标 */
        float V = 1.0f - deltaV * i;
        for(int j = 0; j < lo; ++j)
        {
            /* 计算贴图 U 坐标 */
            float U = 1.0f - deltaU * j;

            /* 计算纬度角 */
            float phi = dLongitude * j;
            float sPh = sin(phi);
            float cPh = cos(phi);

            /* 法向量 */
            vec3 dir = {sTh * cPh, cTh, sTh * sPh};

            /* 填充网格顶点 */
            Vertex vertex = Vertex{radius * dir, dir, {U, V}};
            mesh.Vertices.push_back(vertex);
        }

    }
}

void GBall::setupIndicies(int la, int lo)
{
    // 填充球面
    for (int i = 0; i <= la; ++i)
    {
        int lineAdd = i * lo;
        for (int j = 0; j < lo - 1; ++j)
        {
            // 计算填充块的四个点位置
            int first = lineAdd + j;
            int third = first + lo + 1;
            int second = first + 1;
            int fourth = third - 1;

            // 填充第一块三角形
            mesh.Indices.push_back(first);
            mesh.Indices.push_back(second);
            mesh.Indices.push_back(fourth);

            // 填充第二块三角形
            mesh.Indices.push_back(second);
            mesh.Indices.push_back(third);
            mesh.Indices.push_back(fourth);
        }
    }

    // 填充球面缝合处
    for (int i = 0; i <= la; ++i)
    {
        // 计算填充块的四个点位置
        int second = lo * i;
        int first = second + lo - 1;
        int third = second + lo;
        int fourth = first + lo;

        // 填充第一块三角形
        mesh.Indices.push_back(first);
        mesh.Indices.push_back(second);
        mesh.Indices.push_back(fourth);

        // 填充第二块三角形
        mesh.Indices.push_back(second);
        mesh.Indices.push_back(third);
        mesh.Indices.push_back(fourth);
    }
}

} // namespace GComponent
