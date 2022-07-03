#include "skybox.h"

#include "manager/rendermanager.h"
#include "manager/resourcemanager.h"

namespace GComponent{

const vector<float> SkyBox::VertexData = {
    // positions      triangle order: 0->1->2   |   2->3->0     every stride add 4
    -1.0f,  1.0f, -1.0f,    -1.0f, -1.0f, -1.0f,    1.0f, -1.0f, -1.0f,     1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,    -1.0f, -1.0f, -1.0f,    -1.0f,  1.0f, -1.0f,    -1.0f,  1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,    1.0f, -1.0f,  1.0f,     1.0f,  1.0f,  1.0f,     1.0f,  1.0f, -1.0f, 

    -1.0f, -1.0f,  1.0f,    -1.0f,  1.0f,  1.0f,    1.0f,  1.0f,  1.0f,     1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,    1.0f,  1.0f, -1.0f,     1.0f,  1.0f,  1.0f,     -1.0f,  1.0f,  1.0f,

    -1.0f, -1.0f, -1.0f,    -1.0f, -1.0f,  1.0f,    1.0f, -1.0f, -1.0f,     -1.0f, -1.0f,  1.0f,
};
const vector<string_view> SkyBox::TexturePath = {
    "right.jpg",    "left.jpg",     "top.jpg",
    "bottom.jpg",   "front.jpg",    "back.jpg"
};

SkyBox::SkyBox()
{
    name_   = "skybox";
    mesh_   = "skybox";
    shader_ = "skybox";
    ResourceManager::getInstance().RegisteredMesh(mesh_, new RenderMesh(SetupVertexData(), SetupTrangle(), {}));
}

void SkyBox::tickImpl(float delta_time)
{
    //RenderManager::getInstance().EmplaceAuxiRenderCommand(name_, shader_, mesh_);
    RenderManager::getInstance().EmplaceRenderCommand(name_, shader_, mesh_);
}

void SkyBox::setShaderProperty(MyShader& shader)
{
    shader.setMat4("model", glm::mat4(1.0f));
    shader.setVec3("color", glm::vec3(1.0f, 0.0f, 0.0f));
}

vector<Vertex> SkyBox::SetupVertexData()
{
    vector<Vertex> vertices(VertexData.size() / 3);
    for (int i = 0, idx = 0; i < vertices.size(); ++i, idx+= 3) {        
        vertices[i].Position  = glm::vec3(VertexData[idx], VertexData[idx + 1], VertexData[idx + 2]);
        vertices[i].Normal    = glm::vec3(0.0f);
        vertices[i].TexCoords = glm::vec3(0.0f);
    }
    return vertices;
}

vector<Triangle> SkyBox::SetupTrangle()
{
    vector<Triangle> trangles(12);
    for (int i = 0, idx = 0; i < 12; i += 2, idx += 4) {
        trangles[i]     = Triangle{ idx, idx + 1, idx + 2 };
        trangles[i + 1] = Triangle{ idx + 2, idx + 3, idx };
    }
    return trangles;
}

}