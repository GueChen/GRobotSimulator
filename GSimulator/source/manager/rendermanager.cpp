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
	InitializeIBLResource();
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
	ambient_observer_UBO_->SetSubData(&m_render_sharing_msg.dir_light.intensity,			sizeof glm::vec4 + sizeof glm::vec3,     sizeof(float));		
	ambient_observer_UBO_->SetSubData(glm::value_ptr(m_render_sharing_msg.view_pos),		sizeof glm::vec4 * 2,					 sizeof glm::vec3);	
	ambient_observer_UBO_->SetSubData(&m_render_sharing_msg.projection_info.near_plane,		sizeof glm::vec4 * 2 + sizeof glm::vec3, sizeof(float));
	ambient_observer_UBO_->SetSubData(&m_render_sharing_msg.projection_info.far_plane,		sizeof glm::vec4 * 3,					 sizeof(float));
}

void RenderManager::InitializeIBLResource()
{	
	static glm::mat4 capture_proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.0f, 10.0f);
	static glm::mat4 capture_views[] = {
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f),  glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f,  0.0f),  glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f),  glm::vec3(0.0f,  0.0f, 1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  -1.0f, 0.0f),  glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f),  glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
	};
	
	unsigned int hdr_env     = gl_->LoadTexture("./asset/textures/loft_newport/Newport_Loft_Ref.hdr", false);
	unsigned int environment_cubemap = 0,
				 irradiance_cubemap  = 0,
				 prefilter_cubemap   = 0,
				 brdf_lut			 = 0;
				 
	auto& resources = ResourceManager::getInstance();
	RenderMesh* sky_box_mesh = resources.GetMeshByName(skybox_.getMesh()),
			  * quad_mesh	 = resources.GetMeshByName(screen_quad_.getMesh());
	
	
	
	unsigned fbo, rbo;
	gl_->glGenFramebuffers(1, &fbo);
	gl_->glGenRenderbuffers(1, &rbo);

	gl_->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	gl_->glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	gl_->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

	gl_->glEnable(GL_DEPTH_TEST);
	gl_->glDepthFunc(GL_LEQUAL);
	gl_->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	{				
		//FBOGuard gaurd(&fbo);
		UBOGaurd ubo_gaurd(&matrices_UBO_.value());
		matrices_UBO_->SetSubData(&capture_proj, 0, sizeof glm::mat4);
		int maxTextureSize;
		gl_->glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
		std::cout << maxTextureSize << std::endl;
		uint32_t ibl_width = 1024, ibl_height = 1024;
		gl_->glGenTextures(1, &environment_cubemap);
		gl_->glBindTexture(GL_TEXTURE_CUBE_MAP, environment_cubemap);
		for (uint32_t i = 0; i < 6; ++i) {
			gl_->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, ibl_width, ibl_height, 0, GL_RGB, GL_FLOAT, nullptr);
		}

		gl_->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl_->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		gl_->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		gl_->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl_->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl_->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, ibl_width, ibl_height);

		MyShader* e2c_shader = resources.GetShaderByName("equirectangular2cube");
		gl_->glBindTextureUnit(3, hdr_env);
		gl_->glViewport(0, 0, ibl_width, ibl_height);
		e2c_shader->use();
		for (uint32_t i = 0; i < 6; ++i) {
			matrices_UBO_->SetSubData(&capture_views[i], sizeof glm::mat4, sizeof glm::mat4);
			gl_->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, environment_cubemap, 0);
			gl_->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			sky_box_mesh->Draw();
		}
		
		uint32_t irr_width = 128, irr_height = 128;
		gl_->glGenTextures(1, &irradiance_cubemap);
		gl_->glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_cubemap);
		for (uint32_t i = 0; i < 6; ++i) {
			gl_->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, irr_width, irr_height, 0, GL_RGB, GL_FLOAT, nullptr);
		}

		gl_->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl_->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		gl_->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		gl_->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl_->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl_->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, irr_width, irr_height);

		MyShader* irr_shader = resources.GetShaderByName("irr_conv");
		gl_->glBindTextureUnit(3, environment_cubemap);
		gl_->glViewport(0, 0, irr_width, irr_height);
		irr_shader->use();
		for (uint32_t i = 0; i < 6; ++i) {
			matrices_UBO_->SetSubData(&capture_views[i], sizeof glm::mat4, sizeof glm::mat4);
			gl_->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradiance_cubemap, 0);
			gl_->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			sky_box_mesh->Draw();
		}

		uint32_t pft_width = 128, pft_height = 128;
		gl_->glGenTextures(1, &prefilter_cubemap);
		gl_->glBindTexture(GL_TEXTURE_CUBE_MAP, prefilter_cubemap);
		for (uint32_t i = 0; i < 6; ++i) {
			gl_->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, pft_width, pft_height, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		gl_->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl_->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		gl_->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		gl_->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		gl_->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl_->glGenerateMipmap(GL_TEXTURE_CUBE_MAP);


		MyShader* pft_shader = resources.GetShaderByName("pft_conv");
		pft_shader->use();
		uint32_t max_mipmap_levels = 5;
		for (uint32_t level = 0; level < max_mipmap_levels; ++level) {
			uint32_t mip_width = pft_width * pow(0.5, level);
			uint32_t mip_height = pft_height * pow(0.5, level);
			gl_->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mip_width, mip_height);
			gl_->glViewport(0, 0, mip_width, mip_height);

			float roughness = (float)level / (float)(max_mipmap_levels - 1);
			pft_shader->setFloat("roughness", roughness);
			for (uint32_t i = 0; i < 6; ++i) {
				matrices_UBO_->SetSubData(&capture_views[i], sizeof glm::mat4, sizeof glm::mat4);
				gl_->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilter_cubemap, level);
				gl_->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				sky_box_mesh->Draw();
			}
		}

		uint32_t brdf_width = 512, brdf_height = 512;
		gl_->glGenTextures(1, &brdf_lut);
		gl_->glBindTexture(GL_TEXTURE_2D, brdf_lut);
		gl_->glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, brdf_width, brdf_height, 0, GL_RG, GL_FLOAT, 0);

		gl_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gl_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		gl_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		gl_->glViewport(0, 0, brdf_width, brdf_height);
		gl_->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, brdf_width, brdf_height);
		gl_->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdf_lut, 0);

		MyShader* brdf_shader = resources.GetShaderByName("brdf_lut");
		brdf_shader->use();
		gl_->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		quad_mesh->Draw();

	}

	gl_->glBindTextureUnit(4, irradiance_cubemap);
	gl_->glBindTextureUnit(5, prefilter_cubemap);
	gl_->glBindTextureUnit(6, brdf_lut);
	gl_->glBindTextureUnit(7, environment_cubemap);
	gl_->glDeleteFramebuffers(1,  &fbo);
	gl_->glDeleteRenderbuffers(1, &rbo);
	gl_->glDepthFunc(GL_LESS);
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
	PassSpecifiedListDepth(shadow_cast_list_, [](const std::string& name) {
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

#ifdef _USE_CSM
	MyShader* depth_shader = scene_manager.GetShaderByName("csm_depth_map"); depth_shader->use();
#else 
	MyShader* depth_shader = scene_manager.GetShaderByName("depth_map"); depth_shader->use();
#endif

	for (auto& [obj_name, mesh_name] : list) 
	{		
		RenderMesh* mesh = scene_manager.GetMeshByName(mesh_name);
		Model* obj = ObjGetter(obj_name);

		obj->setShaderProperty(*depth_shader);
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