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
#include "component/material_component.h"

#include <QtGUI/QOpenGLContext>

#include <iostream>

namespace GComponent {
	
/*__________________________PUBLIC METHODS____________________________________*/
RenderManager::~RenderManager() = default;

bool RenderManager::InitFrameBuffer()
{	
	FBO_ = FrameBufferObject(m_render_sharing_msg.viewport.window_size.x,		// window width
							 m_render_sharing_msg.viewport.window_size.y,		// window height
							 gl_);
	
	if (depth_FBO_) {
		gl_->glDeleteFramebuffers(1, &depth_FBO_);
		gl_->glDeleteTextures(1, &depth_texture_);
		depth_FBO_ = depth_texture_ = 0;

	}

	if (not depth_FBO_) 
	{
		gl_->glGenFramebuffers(1, &depth_FBO_);
		gl_->glGenTextures(1, &depth_texture_);
		
		// Configure Texture Properties
		gl_->glBindTexture(GL_TEXTURE_2D, depth_texture_);
		gl_->glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 4096, 4096, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		gl_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		gl_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		gl_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// Bind Texture on Frame Buffer
		gl_->glBindFramebuffer(GL_FRAMEBUFFER, depth_FBO_);
		gl_->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture_, 0);
		gl_->glDrawBuffer(GL_NONE);
		gl_->glReadBuffer(GL_NONE);
		gl_->glBindFramebuffer(GL_FRAMEBUFFER, 0);	
	
	}

	if (m_csm_depth_FBO) {
		gl_->glDeleteFramebuffers(1, &m_csm_depth_FBO);
		gl_->glDeleteTextures(1, &m_csm_depth_texture);
		m_csm_depth_FBO = m_csm_depth_texture = 0;
	}

	if (not m_csm_depth_FBO) 
	{		
		// Generator Handle
		gl_->glGenFramebuffers(1, &m_csm_depth_FBO);
		gl_->glGenTextures(1, &m_csm_depth_texture);

		// Configure Texture Properties
		gl_->glBindTexture(GL_TEXTURE_2D_ARRAY, m_csm_depth_texture);
		gl_->glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, 
					      m_csm_texture_resolusion, m_csm_texture_resolusion, m_csm_levels,
						  0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		gl_->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		gl_->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		gl_->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		gl_->glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		constexpr float bordercolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		gl_->glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, bordercolor);

		// Bind Texture on Frame Buffer
		gl_->glBindFramebuffer(GL_FRAMEBUFFER, m_csm_depth_FBO);
		gl_->glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_csm_depth_texture, 0);
		gl_->glDrawBuffer(GL_NONE);
		gl_->glReadBuffer(GL_NONE);

		int status = gl_->glCheckFramebufferStatus(GL_FRAMEBUFFER);
		// Check Error
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			std::cout << "ERROR::FRAMEBUFFER::CSM_DEPTH_BUFFER:: buffer is not complete!\n";
		}
		gl_->glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// Configure the light space matrices parameters
	if (m_csm_matrices_UBO) {
		gl_->glDeleteBuffers(1, &m_csm_matrices_UBO);
		m_csm_matrices_UBO = 0;
	}

	if (not m_csm_matrices_UBO) {
		gl_->glGenBuffers(1, &m_csm_matrices_UBO);
		gl_->glBindBuffer(GL_UNIFORM_BUFFER, m_csm_matrices_UBO);
		gl_->glBufferData(GL_UNIFORM_BUFFER, sizeof glm::mat4x4 * 16, nullptr, GL_STATIC_DRAW);
		gl_->glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_csm_matrices_UBO);
		gl_->glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	return FBO_ == std::nullopt;	
}

void RenderManager::PushRenderCommand(const RenderCommand & command)
{
	render_list_.push_back(command);
}

void RenderManager::EmplaceRenderCommand(string obj_name, string mesh_name)
{
	render_list_.emplace_back(obj_name, mesh_name);
}

void RenderManager::PushAuxiRenderCommand(const RenderCommand& command)
{
	axui_render_list_.push_back(command);
}

void RenderManager::EmplaceAuxiRenderCommand(string obj_name, string mesh_name)
{
	axui_render_list_.emplace_back(obj_name, mesh_name);
}

void RenderManager::PushPostProcessRenderCommand(const RenderCommand& command)
{
	post_process_list_.push_back(command);
}

void RenderManager::EmplacePostProcessRenderCommand(string obj_name, string mesh_name)
{
	post_process_list_.emplace_back(obj_name, mesh_name);
}

void RenderManager::EmplaceFrontPostProcessRenderCommand(string obj_name, string mesh_name)
{
	post_process_list_.emplace_front(obj_name, mesh_name);
}

void RenderManager::EmplaceAuxiliaryObj(shared_ptr<SimplexModel>&& obj)
{
	// Notice May Exist Concurrency Problems //
	std::lock_guard<std::mutex> lock(planning_lock_);
	planning_aux_lists_.emplace_back(std::forward<decltype(obj)>(obj));
}

void RenderManager::ClearAuxiliaryObj()
{
	// Notice May Exist Concurrency Problems //
	std::lock_guard<std::mutex> lock(planning_lock_);
	delete_count_ = planning_aux_lists_.size();
}

void RenderManager::SetPickingController(PickingController& controller)
{
	picking_controller_handle_ = controller;
}

void RenderManager::SetGL(const shared_ptr<MyGL>& gl)
{
	gl_ = gl;
	InitFrameBuffer();

	if (matrices_UBO_) {
        gl_->glDeleteBuffers(1, (unsigned*) &matrices_UBO_);
        gl_->glDeleteBuffers(1, (unsigned*) &ambient_observer_UBO_);
        matrices_UBO_ = ambient_observer_UBO_ 
                      = 0;         
    }
    matrices_UBO_         = gl_->genMatrices();
    ambient_observer_UBO_ = gl_->genDirLightViewPos();
}

// 渲染从该处开始，所有的 Draw call 由该部分完成
/*__________________________tick Methods____________________________________________________*/
void RenderManager::tick()
{
	SetProjectViewMatrices();
	SetDirLightViewPosition();
	// setting light matrices UBO
	m_csm_cascade_planes = { m_render_sharing_msg.projection_info.far_plane / 50.0f, 
							 m_render_sharing_msg.projection_info.far_plane / 25.0f, 
							 m_render_sharing_msg.projection_info.far_plane / 10.0f, 
							 m_render_sharing_msg.projection_info.far_plane / 2.0f };
	const auto light_view_proj_matrices = GetLightViewProjMatrices();
	gl_->glBindBuffer(GL_UNIFORM_BUFFER, m_csm_matrices_UBO);
	for (size_t i = 0; i < light_view_proj_matrices.size(); ++i) {
		gl_->glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof glm::mat4x4, sizeof glm::mat4x4, &light_view_proj_matrices[i]);
	}
	gl_->glBindBuffer(GL_UNIFORM_BUFFER, 0);
	

	PickingPass();
	
	DepthMapPass();

	NormalPass();
	
	PostProcessPass();

	Clear();
}

/*_____________________________PROTECTED METHODS__________________________________________*/
RenderManager::RenderManager() :grid_(50, 0.20f)
{}

/*_____________________________PRIVATE METHODS____________________________________________*/
//_____________________________Datas Setting______________________________________________________//
void RenderManager::SetProjectViewMatrices()
{
	gl_->setMatrices(matrices_UBO_, 
					 m_render_sharing_msg.projection_mat, 
					 m_render_sharing_msg.view_mat);
}

void RenderManager::SetDirLightViewPosition()
{
	gl_->setDirLightViewPos(ambient_observer_UBO_, 
							m_render_sharing_msg.dir_light.dir, 
							m_render_sharing_msg.dir_light.color,
							m_render_sharing_msg.view_pos);
}

std::vector<glm::vec4> RenderManager::GetFrustumCornersWorldSpace(const glm::mat4& projection, const glm::mat4& view)
{
	const glm::mat4 inv = glm::inverse(projection * view);
	const std::array<float, 2> 
					border_val = {-1.0f, 1.0f};
	std::vector<glm::vec4> frustum_corners;
	for (unsigned x = 0; x < 2; ++x) 
	for (unsigned y = 0; y < 2; ++y)
	for (unsigned z = 0; z < 2; ++z)
	{
		const glm::vec4 pt = inv * glm::vec4(border_val[x], border_val[y], border_val[z], 1.0f);
		frustum_corners.push_back(pt / pt.w);
	}

	return frustum_corners;
}

glm::mat4 RenderManager::GetLightViewProjMatrix(const float near_plane, const float far_plane)
{
	const auto proj_mat		= glm::perspective(glm::radians(m_render_sharing_msg.projection_info.fov), 
											   m_render_sharing_msg.projection_info.aspect, 
											   near_plane, 
											   far_plane);
	const auto frust_points	= GetFrustumCornersWorldSpace(proj_mat, m_render_sharing_msg.view_mat);
	// get the center
	glm::vec3 center  = glm::vec3(0.0f);
	std::for_each(frust_points.begin(), frust_points.end(), [size = frust_points.size(), &center](auto& p) {
		center += 1.0f / size * glm::vec3(p);
	});
	const glm::mat4 light_view = glm::lookAt(center + m_render_sharing_msg.dir_light.dir, 
											 center, 
											 m_render_sharing_msg.GlobalUp);

	// get a bounding box to fit the frustum
	float min_x = std::numeric_limits<float>::max(), min_y = std::numeric_limits<float>::max(), min_z = std::numeric_limits<float>::max(),
		  max_x = std::numeric_limits<float>::min(), max_y = std::numeric_limits<float>::min(), max_z = std::numeric_limits<float>::min();
	for (const auto& v : frust_points) {
		const auto p_in_light_view = light_view * v;
		min_x = std::min(min_x, p_in_light_view.x);
		max_x = std::max(max_x, p_in_light_view.x);
		min_y = std::min(min_y, p_in_light_view.y);
		max_y = std::max(max_y, p_in_light_view.y);
		min_z = std::min(min_z, p_in_light_view.z);
		max_z = std::max(max_z, p_in_light_view.z);
	}

	// modified the z plane
	constexpr float ZMult = 10.0f;
	constexpr float ZInv  = 1.0f / ZMult;
	min_z *= min_z < 0 ? ZMult : ZInv;
	max_z *= max_z < 0 ? ZInv  : ZMult;

	const glm::mat4 light_proj = glm::ortho(min_x, max_x, min_y, max_y, min_z, max_z);
	return light_proj * light_view;
}

std::vector<glm::mat4> RenderManager::GetLightViewProjMatrices()
{
	std::vector<glm::mat4> ret;
	int n = m_csm_cascade_planes.size();
	for (size_t i = 0; i <= n; ++i) 
	{		
		ret.push_back(GetLightViewProjMatrix(
			i == 0 ? m_render_sharing_msg.projection_info.near_plane : m_csm_cascade_planes[i - 1],		// near plane
			i == n ? m_render_sharing_msg.projection_info.far_plane  : m_csm_cascade_planes[i]));		// far plane
	}
	return ret;
}


//_______________________________Rendering Pass____________________________________________________//
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
	// none picking handle no need to picking
	if (!picking_controller_handle_) return;

	PickingGuard picking_guard(picking_controller_handle_.value());
	ClearGLScreenBuffer(0.0f, 0.0f, 0.0f, 1.0f);

	PassSpecifiedListPicking(PassType::DirLightPass, render_list_, [](const std::string& name) {
		return ModelManager::getInstance().GetModelByName(name);
	});
	
	//DisableGuard guard(gl_.get(), GL_DEPTH_TEST);
	gl_->glClear(GL_DEPTH_BUFFER_BIT);
	PassSpecifiedListPicking(PassType::AuxiliaryPass, post_process_list_, [](const std::string& name) {
		return ModelManager::getInstance().GetAuxiModelByName(name);
	});
}

void RenderManager::DepthMapPass()
{			
	gl_->glBindFramebuffer(GL_FRAMEBUFFER, m_csm_depth_FBO);
	gl_->glFramebufferTexture(GL_FRAMEBUFFER, GL_TEXTURE_2D_ARRAY, m_csm_depth_FBO, 0);
	gl_->glViewport(0, 0, m_csm_texture_resolusion, m_csm_texture_resolusion);
	ClearGLScreenBuffer(0.0f, 0.0f, 0.0f, 1.0f);
	gl_->glCullFace(GL_FRONT);
	PassSpecifiedListCSMDepth(render_list_, [](const std::string& name) {
		return ModelManager::getInstance().GetModelByName(name);
	});
	gl_->glCullFace(GL_BACK);
	gl_->glBindFramebuffer(GL_FRAMEBUFFER, QOpenGLContext::currentContext()->defaultFramebufferObject());
	gl_->glViewport(m_render_sharing_msg.viewport.window_pos.x,
					m_render_sharing_msg.viewport.window_pos.y,
					m_render_sharing_msg.viewport.window_size.x,
					m_render_sharing_msg.viewport.window_size.y
	);

	// Shadow Pass
	/*gl_->glViewport(0, 0, 4096, 4096);
	gl_->glBindFramebuffer(GL_FRAMEBUFFER, depth_FBO_);
	ClearGLScreenBuffer(0.0f, 0.0f, 0.0f, 1.0f);
	gl_->glCullFace(GL_FRONT);
	PassSpecifiedListDepth(render_list_, [](const std::string& name) {
		return ModelManager::getInstance().GetModelByName(name);
	});	
	gl_->glCullFace(GL_BACK);
	gl_->glBindFramebuffer(GL_FRAMEBUFFER, 0);
	gl_->glViewport(0, 0, m_width, m_height);*/
}

void RenderManager::NormalPass()
{
	FBOGuard fbo_guard(&FBO_.value());

	ClearGLScreenBuffer(0.0f, 0.0f, 0.05f, 1.0f);
	
	RenderingPass();

	AuxiliaryPass();
	
	SimplexMeshPass();
	
	// TODO: not so good try to hide it
	grid_.SetGL(gl_);
	grid_.Draw();

	gl_->glDepthFunc(GL_LEQUAL);
	skybox_.Draw();
	gl_->glDepthFunc(GL_LESS);
}

void RenderManager::RenderingPass()
{
	// with cascade shadow
	/*PassSpecifiedListCSMShadow(render_list_, [](const std::string& name) {
		return ModelManager::getInstance().GetModelByName(name);
	});*/

	// with shadow
	/*PassSpecifiedListShadow(render_list_, [](const std::string& name) {
		return ModelManager::getInstance().GetModelByName(name);
	});*/
	
	// without shadow
	PassSpecifiedListNormal(render_list_, [](const std::string& name) {
		return ModelManager::getInstance().GetModelByName(name);
		});
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
	for (auto& [obj_name, mesh_name] : list) 
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

	for (auto& [obj_name, mesh_name] : list) 
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

	glm::mat4 light_view	= glm::lookAt(vec3(0.5, 1.0f, 1.0f), vec3(0.0f), vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 light_proj	= glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);
	glm::mat4 light_space	= light_proj * light_view;
	shadow_shader->setMat4("light_space_matrix", light_space);
	shadow_shader->setInt("shadow_map", 0);

	gl_->glBindTexture(GL_TEXTURE_2D, depth_texture_);
	for (auto& [obj_name, mesh_name] : list)
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
	for (auto& [obj_name, mesh_name] : list) 
	{		
		RenderMesh*		mesh	= scene_manager.GetMeshByName(mesh_name);
		Model*			obj		= ObjGetter(obj_name);
		if (!obj || !mesh) continue;		
		auto material = obj->GetComponent<MaterialComponent>(MaterialComponent::type_name.data());		
		if (!material)	continue;
		material->SetShaderProperties();						
		mesh->Draw();	
	}
}

void RenderManager::PassSpecifiedListCSMDepth(RenderList& list, function<RawptrModel(const std::string&)> ObjGetter)
{
	ResourceManager& scene_manager = ResourceManager::getInstance();
	ModelManager&	 model_manager = ModelManager::getInstance();

	MyShader*		 depth_shader = scene_manager.GetShaderByName("csm_depth_map"); depth_shader->use();

	for (auto& [obj_name, mesh_name] : list) 
	{		
		RenderMesh* mesh = scene_manager.GetMeshByName(mesh_name);
		Model*		obj  = ObjGetter(obj_name);

		obj->setShaderProperty(*depth_shader);
		if (mesh) mesh->Draw();
	}
}

void RenderManager::PassSpecifiedListCSMShadow(RenderList& list, function<RawptrModel(const std::string&)> ObjGetter)
{
	ResourceManager& scene_manager	= ResourceManager::getInstance();
	ModelManager&	 model_manager	= ModelManager::getInstance();

	MyShader* shadow_shader			= scene_manager.GetShaderByName("cascade_shadow_ortho"); shadow_shader->use();

	shadow_shader->setInt("shadow_map", 0);
	shadow_shader->setFloat("far_plane",   m_render_sharing_msg.projection_info.far_plane);
	shadow_shader->setInt("csm_levels",	   m_csm_cascade_planes.size());
	for (size_t i = 0; i < m_csm_cascade_planes.size(); ++i) {
		shadow_shader->setFloat("cascade_plane[" + std::to_string(i) + "]", m_csm_cascade_planes[i]);
	}

	gl_->glBindTexture(GL_TEXTURE_2D_ARRAY, m_csm_depth_texture);
	for (auto& [obj_name, mesh_name] : list)
	{
		RenderMesh* mesh = scene_manager.GetMeshByName(mesh_name);
		Model*		obj	 = ObjGetter(obj_name);
		
		obj->setShaderProperty(*shadow_shader);
		if (mesh) {
			mesh->Draw();
		}		
	}
	gl_->glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderManager::SimplexMeshPass()
{
	std::lock_guard<std::mutex> lock(planning_lock_);
	/* clean up before draw */
	for (int i = 0; i < delete_count_; ++i) {
		planning_aux_lists_.pop_front();
	}
	delete_count_ = 0;
	gl_->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	for (auto& obj : planning_aux_lists_) {
		obj->SetGL(gl_);
		obj->Draw(nullptr);
	}
	gl_->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
}

}