#include "basegrid.h"

#include "render/mygl.hpp"
#include "manager/resourcemanager.h"

using namespace GComponent;

BaseGrid::BaseGrid(int n, float size):
    num(n), gridSize(size)
{

}

void BaseGrid::GLBufferInitialize()
{
    /* 避免重复初始化 */
    if(isInit)
    {
        return;
    }
    isInit = true;

    vector<vec3> verts = GetGridVertexLocation(num, gridSize);
    vector<Line> edges = GetGridEdge(num);

    std::tie(VAO, VBO) = gl->genVABO(&verts[0], GCONST::VEC3_SIZE * verts.size());
    EBO = gl->genEBO(edges);
    gl->EnableVertexAttribArrays_continus(verts);

}

void BaseGrid::SetGL(shared_ptr<MyGL> other)
{
    gl = other;
    GLBufferInitialize();
}

void BaseGrid::Draw()
{
    if (MyShader* shader = ResourceManager::getInstance().GetShaderByName("base"); shader) {
        shader->use();
        shader->setMat4("model", mat4(1.0f));
        shader->setBool("normReverse", false);
    }
    gl->glBindVertexArray(VAO);
    gl->glDrawElements(GL_LINES, num * 4, GL_UNSIGNED_INT, 0);
    gl->glBindVertexArray(0);
}

vector<vec3> BaseGrid::GetGridVertexLocation(int num, float size)
{
    vector<vec3> locations((num - 1) * 4);

    auto it = locations.begin();
    const float edgeLen = size * (num - 1);
    const float corner  = -edgeLen * 0.5f;
    const float counter = corner + edgeLen;

    {
        /* 添加横对列 */
        float locationX = corner;
        for(int i = 0; i < num; ++i)
        {
            (*it++) = vec3(locationX, corner, 0.0);
            (*it++) = vec3(locationX, counter, 0.0);
            locationX += size;
        }
        /* 添加纵格点 */
        float locationY = corner +  size;
        for(int i = 1; i < num - 1; ++i)
        {
            (*it++) = vec3(corner,  locationY, 0.0f);
            (*it++) = vec3(counter, locationY, 0.0f);
            locationY += size;
        }
    }

    return locations;
}

vector<Line> BaseGrid::GetGridEdge(int num)
{
    vector<Line> edges((num - 1) * 2);

    {
        int idx = 0;
        for(auto it = edges.begin(); it != edges.end(); ++it, idx+=2)
        {
            *it = {idx, idx + 1};
        }
        edges.push_back({0,  num * 2 - 2});
        edges.push_back({1,  num * 2 - 1});
    }

    return edges;
}
