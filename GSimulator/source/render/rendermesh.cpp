#include "render/rendermesh.h"

using namespace GComponent;

RenderMesh::RenderMesh(const std::vector<Vertex>&   vertices, 
                       const std::vector<Triangle>& indices, 
                       const std::vector<Texture>&  textures)
{
    mesh_datas_.vertices = vertices;
    mesh_datas_.indices  = indices;
    mesh_datas_.textures = textures;
}

// TODO: 需要添加一个引用计数
RenderMesh::~RenderMesh()
{
    CheckClearRhi();
}

void RenderMesh::SetupMesh()
{
    if(!is_setup_) {
    std::vector<Vertex>& Vertices = mesh_datas_.vertices;
    std::vector<Triangle>& Indices = mesh_datas_.indices;

    /* 检查 Vertex 数据是否有误/为空 */
    if(Vertices.empty()) return;

    mesh_ = rhi_device_->CreateMesh(RhiMeshDesc{
        .vertex_data = Vertices.data(),
        .vertex_data_size = Vertices.size() * sizeof(Vertex),
        .vertex_count = Vertices.size(),
        .vertex_stride = sizeof(Vertex),
        .index_data = Indices.data(),
        .index_data_size = Indices.size() * sizeof(Triangle),
        .index_count = Indices.size() * 3,
        .vertex_layout = RhiVertexLayout::PositionNormalTexcoord
    });

    /* 标志位置true */
    is_setup_ = true;
    }
}

void RenderMesh::SetRhiDevice(const std::shared_ptr<IRhiDevice>& rhi_device)
{
    rhi_device_ = rhi_device;
    CheckClearRhi();
    SetupMesh();
}

void RenderMesh::CheckClearRhi()
{
    if(is_setup_)  {
    rhi_device_->DestroyMesh(mesh_);
    mesh_ = {};
    is_setup_ = false;
    }
}

void RenderMesh::Draw()
{
    rhi_device_->DrawMesh(mesh_, RhiPrimitiveTopology::Triangles, static_cast<uint32_t>(3 * mesh_datas_.indices.size()));
}

void GComponent::RenderMesh::SetupRawMesh(const std::vector<Vertex>& vertices, const std::vector<Triangle>& indices, const std::vector<Texture>& textures)
{
    mesh_datas_.vertices = vertices;
    mesh_datas_.indices  = indices;
    mesh_datas_.textures = textures;
    if (rhi_device_) {
        SetupMesh();
    }

}

void GComponent::RenderMesh::SetupRawMesh(RawMesh&& raw_mesh_datas)
{
    mesh_datas_ = std::move(raw_mesh_datas);
    if (rhi_device_) {
        SetupMesh();
    }
}
