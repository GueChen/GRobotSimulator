#include "modelLoader_qgladapter.h"

namespace GComponent 
{
namespace QGL
{
RenderMesh ModelLoader::getMesh(const std::string& resource)
{
    auto&& [Vs, Is] = GComponent::ModelLoader::readFile(resource);
    return RenderMesh(Vs, Is, std::vector<Texture>{});
}

RenderMesh* ModelLoader::getMeshPtr(const std::string& resource)
{
    auto&& [Vs, Is] = GComponent::ModelLoader::readFile(resource);
    return new RenderMesh(Vs, Is, std::vector<Texture>{});
}
}
}