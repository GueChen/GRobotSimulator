#include "basegrid.h"

#include "manager/resourcemanager.h"

using namespace GComponent;

BaseGrid::BaseGrid(int, float size):
    gridSize(size)
{

}

BaseGrid::~BaseGrid()
{
    if (isInit && rhi_device_) {
        rhi_device_->DestroyMesh(mesh_);
    }
}

void BaseGrid::RhiBufferInitialize()
{
    /* avoid repeated initialization */
    if(isInit)
    {
        return;
    }
    isInit = true;

    const auto verts = GetFullscreenQuadVertexLocation();

    vertex_count_ = static_cast<uint32_t>(verts.size());
    mesh_ = rhi_device_->CreateMesh(RhiMeshDesc{
        .vertex_data = verts.data(),
        .vertex_data_size = sizeof(vec3) * verts.size(),
        .vertex_count = verts.size(),
        .vertex_stride = sizeof(vec3),
        .vertex_layout = RhiVertexLayout::Position3
    });
}

void BaseGrid::SetRhiDevice(shared_ptr<IRhiDevice> rhi_device)
{
    rhi_device_ = rhi_device;
    RhiBufferInitialize();
}

void BaseGrid::Draw()
{
    if (!rhi_device_ || !mesh_) return;

    if (MyShader* shader = ResourceManager::getInstance().GetShaderByName("infinite_grid"); shader) {
        shader->use();
        shader->setFloat("gridSize", gridSize);
        shader->setFloat("majorGridSize", gridSize * 5.0f);
        shader->setFloat("fadeDistance", 120.0f);
        shader->setFloat("depthBias", 0.00001f);
        shader->setVec3("minorLineColor", glm::vec3(0.55f, 0.55f, 0.55f));
        shader->setVec3("majorLineColor", glm::vec3(0.85f, 0.85f, 0.85f));
        shader->setVec3("xAxisColor", glm::vec3(0.80f, 0.16f, 0.16f));
        shader->setVec3("yAxisColor", glm::vec3(0.16f, 0.65f, 0.16f));
        rhi_device_->DrawMesh(mesh_, RhiPrimitiveTopology::Triangles, vertex_count_);
    }
}

std::array<vec3, 6> BaseGrid::GetFullscreenQuadVertexLocation()
{
    return {
        vec3(-1.0f, -1.0f, 0.0f),
        vec3( 1.0f, -1.0f, 0.0f),
        vec3( 1.0f,  1.0f, 0.0f),
        vec3( 1.0f,  1.0f, 0.0f),
        vec3(-1.0f,  1.0f, 0.0f),
        vec3(-1.0f, -1.0f, 0.0f)
    };
}
