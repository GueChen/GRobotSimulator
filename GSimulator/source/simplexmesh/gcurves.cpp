#include "gcurves.h"

#include "manager/resourcemanager.h"

#include <algorithm>
#include <execution>

namespace GComponent {

GCurves::GCurves(vector<vec3> poses, vec3 c1, vec3 c2)
{
    if(poses.size() < 2) throw("ERROR Size");

    verteces.resize(poses.size());
    double step = 1.0 / (poses.size() - 1);
    std::transform(std::execution::seq,
                   poses.begin(), poses.end(), verteces.begin(),
                   [c1, c2, t = 0.0, step](auto && input)mutable
    {
        ColorVertex vertex;

        vertex.position = input;
        vertex.Color = glm::mix(c1, c2, t);
        t += step;
        return vertex;
    });

}

void GCurves::Draw(MyShader *)
{
    if(!isInit) return;
    MyShader* shader = ResourceManager::getInstance().GetShaderByName("linecolor");
    shader->use();
    shader->setMat4("model", glm::mat4(1.0f));
    rhi_device_->DrawMesh(mesh_, RhiPrimitiveTopology::LineStrip, static_cast<uint32_t>(verteces.size()));
}

void GCurves::GLBufferInitialize()
{
    if(isInit) return;

    mesh_ = rhi_device_->CreateMesh(RhiMeshDesc{
        .vertex_data = verteces.data(),
        .vertex_data_size = sizeof(ColorVertex) * verteces.size(),
        .vertex_count = verteces.size(),
        .vertex_stride = sizeof(ColorVertex),
        .vertex_layout = RhiVertexLayout::PositionNormalTexcoordColor
    });

}

} // namespace GComponent
