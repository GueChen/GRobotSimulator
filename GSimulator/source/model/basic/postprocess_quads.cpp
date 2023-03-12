#include "model/basic/postprocess_quads.h"

#include "manager/resourcemanager.h"
#include "manager/rendermanager.h"

#include "component/material_component.h"

namespace GComponent {
const std::vector<Vertex>	PostprocessQuads::vertices = 
{
	Vertex{vec3(-1.0f, 1.0f,  0.99999f),	vec3(0.0f, 0.0f, 0.0f), vec2(0.0f, 1.0f)},		// vertex 0
	Vertex{vec3(-1.0f, -1.0f, 0.99999f),	vec3(0.0f, 0.0f, 0.0f), vec2(0.0f, 0.0f)},		// vertex 1
	Vertex{vec3(1.0f, -1.0f,  0.99999f),	vec3(0.0f, 0.0f, 0.0f), vec2(1.0f, 0.0f)},		// vertex 2
	Vertex{vec3(1.0f, 1.0f,   0.99999f),	vec3(0.0f, 0.0f, 0.0f), vec2(1.0f, 1.0f)} 		// vertex 3
};
const std::vector<Triangle> PostprocessQuads::indeces = 
{
	Triangle{0, 1, 2}, Triangle{2, 3, 0}
};

PostprocessQuads::PostprocessQuads()
{
	name_	= "quads";
	mesh_	= "quads";	
	ResourceManager& resource_manager = ResourceManager::getInstance();
	if (not resource_manager.GetMeshByName(mesh_)) {
		resource_manager.RegisteredMesh(mesh_, new RenderMesh(vertices, indeces, {}));
	}
	RegisterComponent(std::make_unique<MaterialComponent>(this, "postprocess", false));
}

void PostprocessQuads::Draw()
{
	ResourceManager& resource_manager = ResourceManager::getInstance();	
	if (auto mat_ptr = GetComponent<MaterialComponent>(MaterialComponent::type_name.data()); mat_ptr) {
		mat_ptr->SetShaderProperties();
	}
	if (RenderMesh* mesh = resource_manager.GetMeshByName(mesh_); mesh) {
		mesh->Draw();
	}
}

void PostprocessQuads::tickImpl(float delta_time)
{
	RenderManager::getInstance().EmplaceFrontRenderCommand(name_, mesh_, RenderManager::PostProcess);
}

void PostprocessQuads::setShaderProperty(MyShader& shader)
{
	shader.setInt("screen_texture", 0);
}
}