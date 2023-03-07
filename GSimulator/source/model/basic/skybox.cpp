#include "skybox.h"

#include "manager/rendermanager.h"
#include "manager/resourcemanager.h"

#include "component/material_component.h"

#define CubemapPrefix(path) "asset/textures/skybox/"#path

namespace GComponent{

const vector<float> SkyBox::Vertices = {
    // positions      triangle order: 0->1->2   |   2->3->0     every stride add 4
    -1.0f,  1.0f, -1.0f,    -1.0f, -1.0f, -1.0f,    1.0f, -1.0f, -1.0f,     1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,    -1.0f, -1.0f, -1.0f,    -1.0f,  1.0f, -1.0f,    -1.0f,  1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,    1.0f, -1.0f,  1.0f,     1.0f,  1.0f,  1.0f,     1.0f,  1.0f, -1.0f, 
    -1.0f, -1.0f,  1.0f,    -1.0f,  1.0f,  1.0f,    1.0f,  1.0f,  1.0f,     1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,    1.0f,  1.0f, -1.0f,     1.0f,  1.0f,  1.0f,     -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,    -1.0f, -1.0f, -1.0f,    1.0f, -1.0f, -1.0f,     1.0f, -1.0f,  1.0f,
};

const vector<string_view> SkyBox::Textures = {
    CubemapPrefix(right.jpg),    CubemapPrefix(left.jpg),     CubemapPrefix(top.jpg),
    CubemapPrefix(bottom.jpg),   CubemapPrefix(front.jpg),    CubemapPrefix(back.jpg)
};

unsigned SkyBox::count = 0;


/*_______________________________________Constructors and Destructors_________________________________*/
SkyBox::SkyBox()
{
    name_   = "skybox";
    mesh_   = "skybox";    
    CheckAndRegisteredResource();
   
    ++count;
}

SkyBox::~SkyBox()
{
    --count;
    if (not count && cube_texture_id_) {
        ResourceManager::getInstance().DeregisteredTexture("skybox");
    }
}

/*_________________________________________Methods*/
void SkyBox::CheckAndRegisteredResource()
{
    if (not ResourceManager::getInstance().GetMeshByName(mesh_))
    {
        ResourceManager::getInstance().RegisteredMesh(mesh_, new RenderMesh(SetupVertexData(), SetupTrangle(), {}));
    }
    if (count == 0) {
        ResourceManager::getInstance().RegisteredCubemap({ "skybox", Textures, "irradiance", &cube_texture_id_ });
    }
    RegisterComponent(std::make_unique<MaterialComponent>(this, "skybox"));
    
}

void SkyBox::Draw()
{
    ResourceManager& resource_manager = ResourceManager::getInstance();
    if (auto mat_ptr = GetComponent<MaterialComponent>(MaterialComponent::type_name.data()); mat_ptr) {
        mat_ptr->SetShaderProperties();
    }
    if (RenderMesh* mesh = resource_manager.GetMeshByName(mesh_); mesh) {
        mesh->Draw();
    }    
}

void SkyBox::tickImpl(float delta_time)
{
    RenderManager::getInstance().EmplaceRenderCommand(name_, mesh_);
}

void SkyBox::setShaderProperty(MyShader& shader)
{
    shader.setInt("cubemap_texture", 0);
    shader.setMat4("model", glm::mat4(1.0f));
    //shader.setVec3("color", glm::vec3(1.0f, 0.0f, 0.0f));
}

vector<Vertex> SkyBox::SetupVertexData()
{
    vector<Vertex> vertices(Vertices.size() / 3);
    for (int i = 0, idx = 0; i < vertices.size(); ++i, idx+= 3) {
        vertices[i].position  = glm::vec3(Vertices[idx], Vertices[idx + 1], Vertices[idx + 2]);
        vertices[i].normal    = glm::vec3(0.0f);
        vertices[i].texcoords = glm::vec3(0.0f);
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