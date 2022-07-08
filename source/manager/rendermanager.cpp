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

#include <QtGUI/QOpenGLContext>

#include <iostream>

namespace GComponent {

/*__________________________PUBLIC METHODS____________________________________*/
RenderManager::~RenderManager() = default;

bool RenderManager::InitFrameBuffer()
{	
	FBO_ = FrameBufferObject(m_width, m_height, gl_);

	if (not depth_FBO_) 
	{
		ResourceManager::getInstance().RegisteredShader("depth_map",	new MyShader(nullptr, PathVert(depthOrtho),		PathFrag(depthOrtho)));
		ResourceManager::getInstance().RegisteredShader("shadow_color", new MyShader(nullptr, PathVert(shadowOrtho),	PathFrag(shadowOrtho)));
		gl_->glGenFramebuffers(1, &depth_FBO_);
		gl_->glGenTextures(1, &depth_texture_);
		
		// Configure Texture Properties
		gl_->glBindTexture(GL_TEXTURE_2D, depth_texture_);
		gl_->glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 4096, 4096, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		gl_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		gl_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		gl_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		gl_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		//gl_->glBindTexture(GL_TEXTURE_2D, 0);

		// Bind Texture on Frame Buffer
		gl_->glBindFramebuffer(GL_FRAMEBUFFER, depth_FBO_);
		gl_->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture_, 0);
		gl_->glDrawBuffer(GL_NONE);
		gl_->glReadBuffer(GL_NONE);
		gl_->glBindFramebuffer(GL_FRAMEBUFFER, 0);		
	}

	return FBO_ == std::nullopt;	
}

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

void RenderManager::PushPostProcessRenderCommand(const RenderCommand& command)
{
	post_process_list_.push_back(command);
}

void RenderManager::EmplacePostProcessRenderCommand(string obj_name, string shader_name, string mesh_name, string uniform_name)
{
	post_process_list_.emplace_back(obj_name, shader_name, mesh_name, uniform_name);
}

void RenderManager::EmplaceFrontPostProcessRenderCommand(string obj_name, string shader_name, string mesh_name, string uniform_name)
{
	post_process_list_.emplace_front(obj_name, shader_name, mesh_name, uniform_name);
}

void RenderManager::SetPickingController(PickingController& controller)
{
	picking_controller_handle_ = controller;
}

void RenderManager::SetGL(const shared_ptr<MyGL>& gl)
{
	gl_ = gl;
	InitFrameBuffer();
}

// 渲染从该处开始，所有的 Draw call 由该部分完成
/*__________________________tick Methods____________________________________________________*/
void RenderManager::tick()
{

	PickingPass();
	
	ShadowPass();

	NormalPass();
	
	PostProcessPass();

	Clear();
}

/*_____________________________PROTECTED METHODS__________________________________________*/
RenderManager::RenderManager() :grid_(50, 0.20f)
{}


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
	post_process_list_.clear();
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
	PassSpecifiedListPicking(PassType::AuxiliaryPass, post_process_list_, [](const std::string& name) {
		return ModelManager::getInstance().GetAuxiModelByName(name);
	});
}

void RenderManager::ShadowPass()
{	
	gl_->glViewport(0, 0, 4096, 4096);
	gl_->glBindFramebuffer(GL_FRAMEBUFFER, depth_FBO_);
	ClearGLScreenBuffer(0.0f, 0.0f, 0.0f, 1.0f);
	PassSpecifiedListDepth(render_list_, [](const std::string& name) {
		return ModelManager::getInstance().GetModelByName(name);
	});	
	gl_->glBindFramebuffer(GL_FRAMEBUFFER, 0);
	gl_->glViewport(0, 0, m_width, m_height);
}

void RenderManager::NormalPass()
{
	FBOGuard fbo_guard(&FBO_.value());

	ClearGLScreenBuffer(0.0f, 0.0f, 0.05f, 1.0f);
	
	RenderingPass();

	AuxiliaryPass();
	
	// TODO: not so good try to solve it
	grid_.SetGL(gl_);
	grid_.Draw();
	skybox_.Draw();
}

void RenderManager::RenderingPass()
{
	// with shadow
	PassSpecifiedListShadow(render_list_, [](const std::string& name) {
		return ModelManager::getInstance().GetModelByName(name);
		});
	// without shadow
	/*PassSpecifiedListNormal(render_list_, [](const std::string& name) {
		return ModelManager::getInstance().GetModelByName(name);
		});*/
}
void RenderManager::AuxiliaryPass()
{
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
	ClearGLScreenBuffer(0.0f, 0.0f, 0.05f, 1.0f);
	{
		FBOTextureGuard guard(&FBO_.value());		
		screen_quad_.Draw();		
	}

	PassSpecifiedListNormal(post_process_list_, [](const std::string& name) {
		return ModelManager::getInstance().GetAuxiModelByName(name);
		});

}

void RenderManager::PassSpecifiedListPicking(PassType draw_index_type, RenderList& list, function<Model* (const std::string&)> ObjGetter)
{
	ResourceManager& scene_manager	= ResourceManager::getInstance();
	ModelManager&	 model_manager	= ModelManager::getInstance();

	// Universal Shader Uniform Attribute Settings
	MyShader*		 picking_shader = scene_manager.GetShaderByName("picking"); picking_shader->use();

	picking_shader->setUint("gDrawIndex", static_cast<unsigned>(draw_index_type));
	
	//  Pass Normally
	for (auto& [obj_name, shader_name, mesh_name, uniform_name] : list) 
	{		
		RenderMesh*		mesh			= scene_manager.GetMeshByName(mesh_name);
		Model*			obj				= ObjGetter(obj_name);
		
		picking_shader->setUint("gModelIndex", obj->model_id_);
		obj->setShaderProperty(*picking_shader);
		if (mesh) mesh->Draw();
	}
}

void RenderManager::PassSpecifiedListDepth(RenderList& list, function<Model* (const std::string&)> ObjGetter)
{
	ResourceManager& scene_manager = ResourceManager::getInstance();
	ModelManager&	 model_manager = ModelManager::getInstance();

	MyShader*		 depth_shader = scene_manager.GetShaderByName("depth_map"); depth_shader->use();

	glm::mat4 light_view	= glm::lookAt(vec3(0.5f, 1.0f, 1.0f), vec3(0.0f), vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 light_proj	= glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);
	glm::mat4 light_space	= light_proj * light_view;
	depth_shader->setMat4("light_space_matrix", light_space);

	for (auto& [obj_name, shader_name, mesh_name, uniform_name] : list) 
	{		
		RenderMesh* mesh = scene_manager.GetMeshByName(mesh_name);
		Model* obj = ObjGetter(obj_name);

		obj->setShaderProperty(*depth_shader);
		if (mesh) mesh->Draw();
	}
}

void RenderManager::PassSpecifiedListShadow(RenderList& list, function<RawptrModel(const std::string&)> ObjGetter)
{
	ResourceManager& scene_manager	= ResourceManager::getInstance();
	ModelManager&	 model_manager	= ModelManager::getInstance();

	MyShader* shadow_shader			= scene_manager.GetShaderByName("shadow_color"); shadow_shader->use();

	glm::mat4 light_view	= glm::lookAt(vec3(0.5f, 1.0f, 1.0f), vec3(0.0f), vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 light_proj	= glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);
	glm::mat4 light_space	= light_proj * light_view;
	shadow_shader->setMat4("light_space_matrix", light_space);
	shadow_shader->setInt("shadow_map", 0);

	gl_->glBindTexture(GL_TEXTURE_2D, depth_texture_);
	for (auto& [obj_name, shader_name, mesh_name, uniform_name] : list)
	{
		RenderMesh* mesh = scene_manager.GetMeshByName(mesh_name);
		Model*		obj	 = ObjGetter(obj_name);

		obj->setShaderProperty(*shadow_shader);
		if (mesh) mesh->Draw();
	}
	gl_->glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderManager::PassSpecifiedListNormal(RenderList& list, std::function<Model* (const std::string&)> ObjGetter)
{
	ResourceManager& scene_manager = ResourceManager::getInstance();
	ModelManager&	 model_manager = ModelManager::getInstance();
	for (auto& [obj_name, shader_name, mesh_name, uniform_name] : list) 
	{
		MyShader*		shader	= scene_manager.GetShaderByName(shader_name);	
		RenderMesh*		mesh	= scene_manager.GetMeshByName(mesh_name);
		Model*			obj		= ObjGetter(obj_name);
		
		shader->use();
		obj->setShaderProperty(*shader);
		if (mesh) mesh->Draw();
	}
}

}