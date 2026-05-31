#include "basegrid.h"

#include "manager/resourcemanager.h"

using namespace GComponent;

BaseGrid::BaseGrid(int n, float size):
    num(n), gridSize(size)
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

    vector<vec3> verts = GetGridVertexLocation(num, gridSize);
    vector<Line> edges = GetGridEdge(num);

    index_count_ = static_cast<uint32_t>(edges.size() * 2);
    mesh_ = rhi_device_->CreateMesh(RhiMeshDesc{
        .vertex_data = verts.data(),
        .vertex_data_size = sizeof(vec3) * verts.size(),
        .vertex_count = verts.size(),
        .vertex_stride = sizeof(vec3),
        .index_data = edges.data(),
        .index_data_size = sizeof(Line) * edges.size(),
        .index_count = index_count_,
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
    if (MyShader* shader = ResourceManager::getInstance().GetShaderByName("base"); shader) {
        shader->use();
        shader->setMat4("model", mat4(1.0f));
        shader->setBool("normReverse", false);
    }
    rhi_device_->DrawMesh(mesh_, RhiPrimitiveTopology::Lines, index_count_);
}

vector<vec3> BaseGrid::GetGridVertexLocation(int num, float size)
{
    vector<vec3> locations((num - 1) * 4);

    auto it = locations.begin();
    const float edgeLen = size * (num - 1);
    const float corner  = -edgeLen * 0.5f;
    const float counter = corner + edgeLen;

    {
        /* vertical lines */
        float locationX = corner;
        for(int i = 0; i < num; ++i)
        {
            (*it++) = vec3(locationX, corner, 0.0);
            (*it++) = vec3(locationX, counter, 0.0);
            locationX += size;
        }
        /* horizontal lines */
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
