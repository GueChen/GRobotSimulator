/**
 *  @file  	rendermanager.cpp
 *  @brief 	The Render Pass Implementations.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	May 7, 2022
 **/
#include "manager/rendermanager.h"

#include "manager/resourcemanager.h"
#include "manager/modelmanager.h"
#include "model/model.h"

#include <iostream>

namespace GComponent {

/*__________________________PUBLIC METHODS____________________________________*/
RenderManager::~RenderManager() = default;

void RenderManager::PushRenderCommand(const RenderCommand & command)
{
	render_list_.push_back(command);
}

void RenderManager::EmplaceRenderCommand(string obj_name, string shader_name, string mesh_name, string uniform_name)
{
	render_list_.emplace_back(obj_name, shader_name, mesh_name, uniform_name);
}

void RenderManager::PushAuxiRenderCommand(const RenderCommand& command)
{
	axui_render_list_.push_back(command);
}

void RenderManager::EmplaceAuxiRenderCommand(string obj_name, string shader_name, string mesh_name, string uniform_name)
{
	axui_render_list_.emplace_back(obj_name, shader_name, mesh_name, uniform_name);
}

void RenderManager::SetPickingController(PickingController& controller)
{
	picking_controller_handle_ = controller;
}

void RenderManager::SetGL(const shared_ptr<MyGL>& gl)
{
	gl_ = gl;
}

void RenderManager::tick()
{
	
	PickingPass();

	ClearGLScreenBuffer(0.0f, 0.0f, 0.2f, 1.0f);

	RenderingPass();

	AuxiliaryPass();

	PostProcessPass();

	Clear();
}

/*_____________________________PROTECTED METHODS__________________________________________*/
RenderManager::RenderManager() = default;


/*_____________________________PRIVATE METHODS____________________________________________*/
void RenderManager::Clear()
{
	ClearList();
	if (picking_controller_handle_) picking_controller_handle_ = std::nullopt;
}

void RenderManager::ClearGLScreenBuffer(float r, float g, float b, float a)
{
	gl_->glEnable(GL_DEPTH_TEST);
	gl_->glClearColor(r, g, b, a);
	gl_->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderManager::ClearList()
{
	render_list_.clear();
	axui_render_list_.clear();
}

void RenderManager::PickingPass()
{
	if (!picking_controller_handle_) return;

	PickingGuard picking_guard(picking_controller_handle_.value());
	ClearGLScreenBuffer(0.0f, 0.0f, 0.0f, 1.0f);

	PassSpecifiedListPicking(PassType::DirLightPass, render_list_, [](const std::string& name) {
		return ModelManager::getInstance().GetModelByName(name);
	});

	DisableGuard guard(gl_.get(), GL_DEPTH_TEST);
	PassSpecifiedListPicking(PassType::AuxiliaryPass, axui_render_list_, [](const std::string& name) {
		return ModelManager::getInstance().GetAuxiModelByName(name);
	});
}

void RenderManager::PassSpecifiedListPicking(PassType draw_index_type, list<RenderCommand>& list, function<Model* (const std::string&)> ObjGetter)
{
	ResourceManager& scene_manager = ResourceManager::getInstance();
	ModelManager& model_manager = ModelManager::getInstance();
	for (auto& [obj_name, shader_name, mesh_name, uniform_name] : list) {
		MyShader*		picking_shader	= scene_manager.GetShaderByName("picking");
		RenderMesh*	mesh			= scene_manager.GetMeshByName(mesh_name);
		Model*			obj				= ObjGetter(obj_name);

		picking_shader->use();
		picking_shader->setUint("gDrawIndex",  static_cast<unsigned>(draw_index_type));
		picking_shader->setUint("gModelIndex", obj->model_id_);
		obj->setShaderProperty(*picking_shader);
		if (mesh) mesh->Draw();
	}
}

void RenderManager::PassSpecifiedListNormal(std::list<RenderCommand>& list, std::function<Model* (const std::string&)> ObjGetter)
{
	ResourceManager& scene_manager = ResourceManager::getInstance();
	ModelManager& model_manager = ModelManager::getInstance();
	for (auto& [obj_name, shader_name, mesh_name, uniform_name] : list) {

		MyShader*      shader	= scene_manager.GetShaderByName(shader_name);	
		RenderMesh* mesh		= scene_manager.GetMeshByName(mesh_name);
		Model*         obj		= ObjGetter(obj_name);

		shader->use();
		obj->setShaderProperty(*shader);
		if (mesh) mesh->Draw();
	}
}

void RenderManager::RenderingPass()
{
	PassSpecifiedListNormal(render_list_, [](const std::string& name) {
		return ModelManager::getInstance().GetModelByName(name);
		});
}
void RenderManager::AuxiliaryPass()
{
	DisableGuard guard(gl_.get(), GL_DEPTH_TEST);
	PassSpecifiedListNormal(axui_render_list_, [](const std::string& name) {
		return ModelManager::getInstance().GetAuxiModelByName(name);
		});
}

void RenderManager::PostProcessPass()
{
	//TODO: add some postprocess effect
	// 1. draw selected object
	// 2. tone mapping & color grading
	// 3. ambient occlusion
}
}