#include "manager/rendermanager.h"

#include "manager/scenemanager.h"
#include "manager/modelmanager.h"
#include "model/model.h"


namespace GComponent {
RenderManager::RenderManager() = default;

RenderManager::~RenderManager() = default;

void RenderManager::PushRenderCommand(const RenderCommand & command)
{
	render_list_.push_back(command);
}

void RenderManager::EmplaceRenderCommand(string obj_name, string shader_name, string mesh_name, string uniform_name)
{
	render_list_.emplace_back(obj_name, shader_name, mesh_name, uniform_name);
}

void RenderManager::SetGL(const shared_ptr<MyGL>& gl)
{
	gl_ = gl;
}

void RenderManager::tick()
{
	
	//ClearGL();

	PickingPass();

	//ClearGL();

	RenderingPass();

	AuxiliaryPass();

	//ClearGL();

	PostProcessPass();
}


void RenderManager::ClearGL()
{
	gl_->glEnable(GL_DEPTH_TEST);
	gl_->glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
	gl_->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderManager::PickingPass()
{
	SceneManager& scene_manager = SceneManager::getInstance();
	ModelManager& model_manager = ModelManager::getInstance();
}

void RenderManager::RenderingPass()
{
	SceneManager& scene_manager = SceneManager::getInstance();
	ModelManager& model_manager = ModelManager::getInstance();
	for (auto& [obj_name, shader_name, mesh_name, uniform_name] : render_list_) {

		MyShader*	    shader	= scene_manager.GetShaderByName(shader_name);
		Model*			obj		= model_manager.GetModelByName(obj_name);
		MeshComponent*	mesh	= scene_manager.GetMeshByName(mesh_name);
		
		shader->use();
		shader->setMat4("model", obj->getModelMatrix());
		obj->setShaderProperty(*shader);
		if(mesh) mesh->Draw();
		// TODO: For other Shader Bind Value Pass
	}
	render_list_.clear();
}
void RenderManager::AuxiliaryPass()
{
	// TODO: draw Auxiliary render obj
	// 1. draw axis
	// 2. draw simplex mesh
}

void RenderManager::PostProcessPass()
{
	//TODO: add some postprocess effect
	// 1. draw selected object
	// 2. tone mapping & color grading
	// 3. ambient occlusion
}
}