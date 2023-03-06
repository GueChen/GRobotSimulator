#include "modelLoader_qgladapter.h"

#include <exception>

namespace GComponent 
{
namespace QGL
{
RenderMesh ModelLoader::getMesh(const std::string& resource)
{
    auto&& [Vs, Is] = GComponent::ModelLoader::ReadFile(resource);    
    return RenderMesh(Vs, Is, std::vector<Texture>{});
}

RenderMesh* ModelLoader::getMeshPtr(const std::string& resource)
{
    auto&& [Vs, Is] = GComponent::ModelLoader::ReadFile(resource);
    return Vs.empty()? nullptr: new RenderMesh(Vs, Is, std::vector<Texture>{});
}
}
}