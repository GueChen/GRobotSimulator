#include "modelLoader_qgladapter.h"

namespace GComponent 
{
namespace QGL
{
MeshComponent ModelLoader::getMesh(const std::string& resource)
{
    auto&& [Vs, Is] = GComponent::ModelLoader::readFile(resource);
    return MeshComponent(Vs, Is, std::vector<Texture>{});
}

MeshComponent* ModelLoader::getMeshPtr(const std::string& resource)
{
    auto&& [Vs, Is] = GComponent::ModelLoader::readFile(resource);
    return new MeshComponent(Vs, Is, std::vector<Texture>{});
}
}
}