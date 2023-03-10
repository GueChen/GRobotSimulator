#include "model/basic/postprocess_quads.h"

#include "manager/resourcemanager.h"
#include "manager/rendermanager.h"

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
	shader_ = "postprocess";
	ResourceManager& resource_manager = ResourceManager::getInstance();
	if (not resource_manager.GetMeshByName(mesh_)) {
		resource_manager.RegisteredMesh(mesh_, new RenderMesh(vertices, indeces, {}));
	}
	/*if (not resource_manager.GetShaderByName(shader_)) {
		resource_manager.RegisteredShader(shader_, new MyShader(nullptr, PathVert(postprocess), PathFrag(postprocess)));
	}*/
}

void PostprocessQuads::Draw()
{
	ResourceManager& resource_manager = ResourceManager::getInstance();		
	if (MyShader* shader = resource_manager.GetShaderByName(shader_); shader) {
		shader->link();
		shader->use();
		setShaderProperty(*shader);
	}	
	if (RenderMesh* mesh = resource_manager.GetMeshByName(mesh_); mesh) {
		mesh->Draw();
	}
}

void PostprocessQuads::tickImpl(float delta_time)
{
	RenderManager::getInstance().EmplaceFrontPostProcessRenderCommand(name_, shader_, mesh_);
}

void PostprocessQuads::setShaderProperty(MyShader& shader)
{
	//shader.setUniformValue("screen_texture", 0);
	shader.setInt("screen_texture", 0);
}
}