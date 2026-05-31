#include "gline.h"

#include "manager/resourcemanager.h"

#include <cstdint>
#include <vector>

namespace GComponent {

GLine::GLine(vec3 p1, vec3 p2, vec3 c1, vec3 c2)
{
    vert_begin.position = p1;
    vert_begin.Color    = c1;

    vert_end.position   = p2;
    vert_end.Color      = c2;
   
}

void GLine::Draw(MyShader *)
{
    // TODO: 考虑抛出异常提示初始化？
    if(!isInit) return;
    MyShader* shader = ResourceManager::getInstance().GetShaderByName("linecolor");
    if (!shader) return;
    shader->use();
    shader->setMat4("model", glm::mat4(1.0f));
    rhi_device_->DrawMesh(mesh_, RhiPrimitiveTopology::Lines, 2);
}

void GLine::GLBufferInitialize()
{
    if(isInit) return;

    const std::vector<ColorVertex> vertices{ vert_end, vert_begin };
    const std::vector<uint32_t> indices{ 0, 1 };
    mesh_ = rhi_device_->CreateMesh(RhiMeshDesc{
        .vertex_data = vertices.data(),
        .vertex_data_size = sizeof(ColorVertex) * vertices.size(),
        .vertex_count = vertices.size(),
        .vertex_stride = sizeof(ColorVertex),
        .index_data = indices.data(),
        .index_data_size = sizeof(uint32_t) * indices.size(),
        .index_count = indices.size(),
        .vertex_layout = RhiVertexLayout::PositionNormalTexcoordColor
    });
}

} // namespace GComponent
