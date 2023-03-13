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

void RenderManager::InitFrameBuffer()
{	
	render_FBO_ = FrameBufferObject(m_render_sharing_msg.viewport.window_size.x,		// window width
							 m_render_sharing_msg.viewport.window_size.y,		// window height
							 FrameBufferObject::Color, gl_);	
}

void RenderManager::EmplaceRenderCommand(std::string obj_name, std::string mesh_name, QueueType type)
{
	switch (type) {
	case Normal:
		render_list_.emplace_back(obj_name, mesh_name);
		break;
	case PostProcess:
		post_process_list_.emplace_back(obj_name, mesh_name);
		break;
	case Depth:
		shadow_cast_list_.emplace_back(obj_name, mesh_name);
		break;
	}
}

void RenderManager::EmplaceFrontRenderCommand(std::string obj_name, std::string mesh_name, QueueType type)
{
	switch (type) {
	case Normal:
		render_list_.emplace_front(obj_name, mesh_name);
		break;
	case PostProcess:
		post_process_list_.emplace_front(obj_name, mesh_name);
		break;
	case Depth:
		shadow_cast_list_.emplace_front(obj_name, mesh_name);
		break;
	}
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
#ifdef _USE_CSM
	depth_FBO_ = FrameBufferObject(depth_buffer_resolustion_,
								   depth_buffer_resolustion_,
								   m_csm_levels,
								   FrameBufferObject::Depth,
								   gl_);
#else	
	depth_FBO_ = FrameBufferObject(depth_buffer_resolustion_, 
								   depth_buffer_resolustion_, 
								   FrameBufferObject::Depth,
								   gl_);
#endif

	matrices_UBO_		  = UniformBufferObject(0, sizeof glm::mat4x4 * 2,  gl_);
	ambient_observer_UBO_ = UniformBufferObject(1, 
												sizeof glm::vec4 * 3 + sizeof(float),  
												gl_);
	light_matrices_UBO_	  = UniformBufferObject(2, 
												sizeof glm::mat4x4 * 16 + sizeof glm::vec4 * 16 + sizeof(unsigned int),
												gl_);
}

// 渲染从该处开始，所有的 Draw call 由该部分完成
/*__________________________tick Methods____________________________________________________*/
void RenderManager::tick()
{
	SetProjectViewMatrices ();
	SetDirLightViewPosition();
	// setting light matrices UBO
	m_csm_cascade_planes = {
							 m_render_sharing_msg.projection_info.far_plane / 100.0f,
							 m_render_sharing_msg.projection_info.far_plane / 75.0f,
							 m_render_sharing_msg.projection_info.far_plane / 20.0f,
							 m_render_sharing_msg.projection_info.far_plane / 2.0f };
	const auto light_view_proj_matrices = GetLightViewProjMatrices();
	{
		UBOGaurd gaurd(&light_matrices_UBO_.value());
		light_matrices_UBO_->SetSubData(light_view_proj_matrices.data(), 0, sizeof glm::mat4 * light_view_proj_matrices.size());
		for (int i = 0; i < m_csm_cascade_planes.size(); ++i) {
			light_matrices_UBO_->SetSubData(&m_csm_cascade_planes[i], sizeof glm::mat4 * 16 + sizeof glm::vec4 * i, sizeof(float));
		}		
		light_matrices_UBO_->SetSubData(&m_csm_levels, sizeof glm::mat4 * 16 + sizeof glm::vec4 * 16, sizeof(unsigned int));		
	}		
	/*direct_light_shadow_UBO_->SetData(&shadow_map_pos_, sizeof(uint32_t));*/
	gl_->glBindTextureUnit(3, depth_FBO_->GetTextureID());

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
	UBOGaurd gaurd(&matrices_UBO_.value());
	
	matrices_UBO_->SetSubData(glm::value_ptr(m_render_sharing_msg.projection_mat), 0,				 sizeof glm::mat4);
	matrices_UBO_->SetSubData(glm::value_ptr(m_render_sharing_msg.view_mat),       sizeof glm::mat4, sizeof glm::mat4);
	
}

void RenderManager::SetDirLightViewPosition()
{
	UBOGaurd gaurd(&ambient_observer_UBO_.value());
	
	ambient_observer_UBO_->SetSubData(glm::value_ptr(m_render_sharing_msg.dir_light.dir),   0,										 sizeof glm::vec3);
	ambient_observer_UBO_->SetSubData(glm::value_ptr(m_render_sharing_msg.dir_light.color), sizeof glm::vec4,						 sizeof glm::vec3);
	ambient_observer_UBO_->SetSubData(glm::value_ptr(m_render_sharing_msg.view_pos),		sizeof glm::vec4 * 2,					 sizeof glm::vec3);	
	ambient_observer_UBO_->SetSubData(&m_render_sharing_msg.projection_info.near_plane,		sizeof glm::vec4 * 2 + sizeof glm::vec3, sizeof(float));
	ambient_observer_UBO_->SetSubData(&m_render_sharing_msg.projection_info.far_plane,		sizeof glm::vec4 * 3,					 sizeof(float));
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
	shadow_cast_list_.clear();
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
		
	gl_->glClear(GL_DEPTH_BUFFER_BIT);
	PassSpecifiedListPicking(PassType::AuxiliaryPass, post_process_list_, [](const std::string& name) {
		return ModelManager::getInstance().GetAuxiModelByName(name);
	});
}

void RenderManager::DepthMapPass()
{				
	FBOGuard gaurd(&depth_FBO_.value());	

	gl_->glViewport(0, 0, depth_buffer_resolustion_, depth_buffer_resolustion_);
	ClearGLScreenBuffer(0.0f, 0.0f, 0.0f, 1.0f);	
	gl_->glCullFace(GL_FRONT);

#ifdef _USE_CSM
	PassSpecifiedListCSMDepth(shadow_cast_list_, [](const std::string& name) {
		return ModelManager::getInstance().GetModelByName(name);
	});
#else
	PassSpecifiedListDepth(render_list_, [](const std::string& name) {
		return ModelManager::getInstance().GetModelByName(name);
	});	
#endif
	gl_->glCullFace(GL_BACK);	
	gl_->glViewport(m_render_sharing_msg.viewport.window_pos.x,
					m_render_sharing_msg.viewport.window_pos.y,
					m_render_sharing_msg.viewport.window_size.x,
					m_render_sharing_msg.viewport.window_size.y
	);		
}

void RenderManager::NormalPass()
{
	FBOGuard fbo_guard(&render_FBO_.value());

	ClearGLScreenBuffer(0.0f, 0.0f, 0.05f, 1.0f);
	
	RenderingPass();
	
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
#ifdef _USE_CSM
	PassSpecifiedListNormal(render_list_, [](const std::string& name) {
		return ModelManager::getInstance().GetModelByName(name);
	});
#else
	// with shadow
	PassSpecifiedListShadow(render_list_, [](const std::string& name) {
		return ModelManager::getInstance().GetModelByName(name);
	});
#endif
}

void RenderManager::PostProcessPass()
{
	//TODO: add some postprocess effect
	// 1. draw selected object
	// 2. tone mapping & color grading
	// 3. ambient occlusion
	ClearGLScreenBuffer(0.0f, 0.0f, 0.05f, 1.0f);
	{
		FBOTextureGuard guard(&render_FBO_.value());		
		screen_quad_.Draw();		
	}
	
	gl_->glEnable(GL_BLEND);
	gl_->glCullFace(GL_FRONT);
	gl_->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	PassSpecifiedListNormal(post_process_list_, [](const std::string& name) {
		return ModelManager::getInstance().GetAuxiModelByName(name);
	});

	gl_->glCullFace(GL_BACK);
	PassSpecifiedListNormal(post_process_list_, [](const std::string& name) {
		return ModelManager::getInstance().GetAuxiModelByName(name);
	});
	gl_->glDisable(GL_BLEND);
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

	glm::mat4 light_view	= glm::lookAt(m_render_sharing_msg.dir_light.dir, vec3(0.0f), vec3(0.0f, 0.0f, 1.0f));
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

	glm::mat4 light_view	= glm::lookAt(m_render_sharing_msg.dir_light.dir, vec3(0.0f), vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 light_proj	= glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);
	glm::mat4 light_space	= light_proj * light_view;
	shadow_shader->setMat4("light_space_matrix", light_space);
	shadow_shader->setInt("shadow_map", 0);
	
	FBOTextureGuard guard(&depth_FBO_.value(), GL_TEXTURE0);
	for (auto& [obj_name, mesh_name] : list)
	{
		RenderMesh* mesh = scene_manager.GetMeshByName(mesh_name);
		Model*		obj	 = ObjGetter(obj_name);

		obj->setShaderProperty(*shadow_shader);
		if (mesh) mesh->Draw();
	}	
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