/**
 *  @file  	rendermanager.cpp
 *  @brief 	The Render Pass Implementations.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	May 7, 2022
 **/
#include "manager/rendermanager.h"

#include "manager/resourcemanager.h"
#include "manager/modelmanager.h"
#include "render/rhi/opengl/opengl_ibl_precompute.h"

#include "model/model.h"
#include "component/material_component.h"
#include "component/transform_component.h"

#ifdef _COLLISION_TEST
#include "component/collider_component.h"
#include "geometry/bounding_volume_hierarchy.h"
#endif

#include <QtGUI/QOpenGLContext>

#include <iostream>
#include <stdexcept>

namespace GComponent {
	
/*__________________________PUBLIC METHODS____________________________________*/
RenderManager::~RenderManager() = default;

void RenderManager::InitFrameBuffer()
{	
	render_FBO_ = FrameBufferObject(m_render_sharing_msg.viewport.window_size.x,// window width
							 m_render_sharing_msg.viewport.window_size.y,		// window height
							 FrameBufferObject::Color, rhi_device_);	
	selected_outline_FBO_ = FrameBufferObject(m_render_sharing_msg.viewport.window_size.x,
							 m_render_sharing_msg.viewport.window_size.y,
							 FrameBufferObject::Color, rhi_device_);
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

void RenderManager::SetRhiDevice(const shared_ptr<IRhiDevice>& rhi_device)
{
	rhi_device_ = rhi_device;

	InitFrameBuffer();
#ifdef _USE_CSM
	depth_FBO_ = FrameBufferObject(depth_buffer_resolustion_,
								   depth_buffer_resolustion_,
								   m_csm_levels,
								   FrameBufferObject::Depth,
								   rhi_device_);
#else	
	depth_FBO_ = FrameBufferObject(depth_buffer_resolustion_, 
								   depth_buffer_resolustion_, 
								   FrameBufferObject::Depth,
								   rhi_device_);
#endif

	matrices_UBO_		  = UniformBufferObject(0, sizeof glm::mat4x4 * 2,  rhi_device_);
	ambient_observer_UBO_ = UniformBufferObject(1, 
												sizeof glm::vec4 * 3 + sizeof(float),  
												rhi_device_);
	light_matrices_UBO_	  = UniformBufferObject(2, 
												sizeof glm::mat4x4 * 16 + sizeof glm::vec4 * 16 + sizeof(unsigned int),
												rhi_device_);
	InitializeIBLResource();
}

// Rendering starts here; all draw calls are submitted from this stage.
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
	rhi_device_->BindTextureUnit(3, RhiTextureHandle{ depth_FBO_->GetTextureID() });

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
	auto& resources = ResourceManager::getInstance();
	RenderMesh* sky_box_mesh = resources.GetMeshByName(skybox_.getMesh()),
			  * quad_mesh	 = resources.GetMeshByName(screen_quad_.getMesh());
	MyShader* e2c_shader = resources.GetShaderByName("equirectangular2cube");
	MyShader* irr_shader = resources.GetShaderByName("irr_conv");
	MyShader* pft_shader = resources.GetShaderByName("pft_conv");
	MyShader* brdf_shader = resources.GetShaderByName("brdf_lut");
	if (!sky_box_mesh || !quad_mesh || !e2c_shader || !irr_shader || !pft_shader || !brdf_shader) {
		BindOpenGLFallbackIblResources(rhi_device_, "IBL precompute mesh or shader resources are not initialized");
		return;
	}
	RunOpenGLIblPrecompute(
		rhi_device_,
		matrices_UBO_.value(),
		*sky_box_mesh,
		*quad_mesh,
		*e2c_shader,
		*irr_shader,
		*pft_shader,
		*brdf_shader,
		"./asset/textures/loft_newport/Newport_Loft_Ref.hdr");
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
	rhi_device_->Enable(RhiCapability::DepthTest);
	rhi_device_->SetClearColor(RhiClearColor{ r, g, b, a });
	rhi_device_->Clear(RhiClearFlags::Color | RhiClearFlags::Depth);
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
		
	rhi_device_->Clear(RhiClearFlags::Depth);
	PassSpecifiedListPicking(PassType::AuxiliaryPass, post_process_list_, [](const std::string& name) {
		return ModelManager::getInstance().GetAuxiModelByName(name);
	});
}

void RenderManager::DepthMapPass()
{				
	FBOGuard gaurd(&depth_FBO_.value());	

	rhi_device_->SetViewport(RhiViewport{ 0, 0, depth_buffer_resolustion_, depth_buffer_resolustion_ });
	ClearGLScreenBuffer(0.0f, 0.0f, 0.0f, 1.0f);	
	rhi_device_->SetCullFace(RhiCullFace::Front);

#ifdef _USE_CSM
	PassSpecifiedListDepth(shadow_cast_list_, [](const std::string& name) {
		return ModelManager::getInstance().GetModelByName(name);
	});
#else
	PassSpecifiedListDepth(render_list_, [](const std::string& name) {
		return ModelManager::getInstance().GetModelByName(name);
	});	
#endif
	rhi_device_->SetCullFace(RhiCullFace::Back);	
	rhi_device_->SetViewport(RhiViewport{
		static_cast<int>(m_render_sharing_msg.viewport.window_pos.x),
		static_cast<int>(m_render_sharing_msg.viewport.window_pos.y),
		static_cast<int>(m_render_sharing_msg.viewport.window_size.x),
		static_cast<int>(m_render_sharing_msg.viewport.window_size.y)
	});		
}

void RenderManager::NormalPass()
{
	FBOGuard fbo_guard(&render_FBO_.value());

	ClearGLScreenBuffer(0.0f, 0.0f, 0.05f, 1.0f);
	
	RenderingPass();
	
	SimplexMeshPass();
	
#ifdef _COLLISION_TEST
	rhi_device_->SetPolygonMode(RhiPolygonMode::Line);
	rhi_device_->SetBlendAlpha();
	rhi_device_->Enable(RhiCapability::Blend);
	CollisionPass(render_list_, [](const std::string& name) {
		return ModelManager::getInstance().GetModelByName(name);
	});
	rhi_device_->Disable(RhiCapability::Blend);	
	rhi_device_->SetPolygonMode(RhiPolygonMode::Fill);
#define _DRAW_DBOUNDING_BOX
#ifdef _DRAW_DBOUNDING_BOX
	BoundingBoxPass(render_list_, [](const std::string& name) {
		return ModelManager::getInstance().GetModelByName(name);
	});
#endif
#endif

	// TODO: not so good try to hide it
	rhi_device_->SetDepthFunc(RhiDepthFunc::LessEqual);
	skybox_.Draw();
	rhi_device_->SetDepthFunc(RhiDepthFunc::Less);

	rhi_device_->Enable(RhiCapability::Blend);	
	rhi_device_->SetBlendAlpha();
	grid_.SetRhiDevice(rhi_device_);
	grid_.Draw();
	rhi_device_->Disable(RhiCapability::Blend);
	
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

void RenderManager::SelectedOutlinePass()
{
	if (!selected_outline_FBO_) return;

	FBOGuard outline_guard(&selected_outline_FBO_.value());
	ClearGLScreenBuffer(0.0f, 0.0f, 0.0f, 1.0f);

	if (m_selected_id == 0) return;

	Model* selected_obj = ModelManager::getInstance().GetModelByHandle(m_selected_id);
	if (!selected_obj) return;

	RenderMesh* mesh = ResourceManager::getInstance().GetMeshByName(selected_obj->getMesh());
	MyShader* outline_shader = ResourceManager::getInstance().GetShaderByName("outline");
	if (!mesh || !outline_shader) return;

	auto* transform = selected_obj->GetComponent<TransformComponent>();
	if (!transform) return;

	const glm::mat4 model = Conversion::fromMat4f(transform->GetModelGlobal());

	outline_shader->use();
	outline_shader->setMat4("model", model);
	rhi_device_->Disable(RhiCapability::CullFace);
	mesh->Draw();
}

void RenderManager::PostProcessPass()
{
	//TODO: add some postprocess effect
	// 1. draw selected object
	// 2. tone mapping & color grading
	// 3. ambient occlusion
	SelectedOutlinePass();

	ClearGLScreenBuffer(0.0f, 0.0f, 0.05f, 1.0f);
	{
		FBOTextureGuard color_guard(&render_FBO_.value(), 8);
		if (selected_outline_FBO_) {
			FBOTextureGuard outline_guard(&selected_outline_FBO_.value(), 9);
			screen_quad_.Draw();
		}
		else {
			screen_quad_.Draw();
		}
	}
	
	rhi_device_->Enable(RhiCapability::Blend);
	rhi_device_->SetCullFace(RhiCullFace::Front);
	rhi_device_->SetBlendAlpha();
	PassSpecifiedListNormal(post_process_list_, [](const std::string& name) {
		return ModelManager::getInstance().GetAuxiModelByName(name);
	});

	rhi_device_->SetCullFace(RhiCullFace::Back);
	PassSpecifiedListNormal(post_process_list_, [](const std::string& name) {
		return ModelManager::getInstance().GetAuxiModelByName(name);
	});
	rhi_device_->Disable(RhiCapability::Blend);
}

void RenderManager::PassSpecifiedListPicking(PassType draw_index_type, RenderList& list, function<Model* (const std::string&)> ObjGetter)
{
	ResourceManager& scene_manager	= ResourceManager::getInstance();
	ModelManager&	 model_manager	= ModelManager::getInstance();

	// Universal Shader Uniform Attribute Settings
	MyShader*		 picking_shader = scene_manager.GetShaderByName("picking");
	if (!picking_shader) {
		static bool logged_missing_picking_shader = false;
		if (!logged_missing_picking_shader) {
			std::cerr << "Picking pass skipped: picking shader is not available\n";
			logged_missing_picking_shader = true;
		}
		return;
	}
	picking_shader->use();

	picking_shader->setUint("gDrawIndex", static_cast<unsigned>(draw_index_type));
	
	//  Pass Normally
	for (auto& [obj_name, mesh_name] : list) 
	{		
		RenderMesh*	mesh  = scene_manager.GetMeshByName(mesh_name);
		Model*		obj	  = ObjGetter(obj_name);
		auto&		trans = *obj->GetComponent<TransformComponent>();
		picking_shader->setUint("gModelIndex", obj->model_id_);
		picking_shader->setMat4("model",	   Conversion::fromMat4f(trans.GetModelGlobal()));
		
		if (mesh) mesh->Draw();
	}
}

void RenderManager::PassSpecifiedListDepth(RenderList& list, function<Model* (const std::string&)> ObjGetter)
{
	ResourceManager& scene_manager = ResourceManager::getInstance();
	ModelManager&	 model_manager = ModelManager::getInstance();

#ifdef _USE_CSM
	MyShader* depth_shader = scene_manager.GetShaderByName("csm_depth_map");
#else 
	MyShader* depth_shader = scene_manager.GetShaderByName("depth_map");
#endif
	if (!depth_shader) {
		static bool logged_missing_depth_shader = false;
		if (!logged_missing_depth_shader) {
			std::cerr << "Depth pass skipped: depth shader is not available\n";
			logged_missing_depth_shader = true;
		}
		return;
	}
	depth_shader->use();

	for (auto& [obj_name, mesh_name] : list) 
	{		
		RenderMesh* mesh = scene_manager.GetMeshByName(mesh_name);
		Model* obj = ObjGetter(obj_name);
		auto& trans = *obj->GetComponent<TransformComponent>();
		depth_shader->setMat4("model", Conversion::fromMat4f(trans.GetModelGlobal()));		

		if (mesh) mesh->Draw();
	}
}

#ifdef _COLLISION_TEST
void RenderManager::CollisionPass(RenderList&list, function<RawptrModel(const std::string&)> ObjGetter)
{
	ResourceManager& scene_manager = ResourceManager::getInstance();
	MyShader*		 base_shader   = scene_manager.GetShaderByName("base");
	if (!base_shader) {
		static bool logged_missing_base_shader = false;
		if (!logged_missing_base_shader) {
			std::cerr << "Collision pass skipped: base shader is not available\n";
			logged_missing_base_shader = true;
		}
		return;
	}
	base_shader->use();
	
	for (auto& [obj_name, mesh_name] : list) {
		RenderMesh* mesh = scene_manager.GetMeshByName(mesh_name);
		Model*      obj  = ObjGetter(obj_name);
		if (!obj || !mesh) continue;
		auto& trans = *obj->GetComponent<TransformComponent>();
		base_shader->setMat4("model", Conversion::fromMat4f(trans.GetModelGlobal()));
		if (obj->intesection_) {
			rhi_device_->Disable(RhiCapability::DepthTest);
			rhi_device_->SetCullFace(RhiCullFace::Front);
			rhi_device_->SetLineWidth(0.5f);
			mesh->Draw();
			rhi_device_->Enable(RhiCapability::DepthTest);
			rhi_device_->SetLineWidth(2.5f);
			rhi_device_->SetCullFace(RhiCullFace::Back);
			mesh->Draw();
			rhi_device_->SetLineWidth(1.0f);

		}
	}
}

void RenderManager::BoundingBoxPass(RenderList& list, function<RawptrModel(const std::string&)> ObjGetter)
{
	GLineBox box(vec3(-1.0f), vec3(1.0f));
	box.SetRhiDevice(rhi_device_);
	BoundingBox large;
	std::vector<BoundingBox> boundings;
	for (auto& [obj_name, _] : list) {
		Model* obj = ObjGetter(obj_name);
		if (!obj) continue;
		auto col = obj->GetComponent<ColliderComponent>();
		if (!col) continue;
		const auto& bound = col->GetBound();
		boundings.push_back(bound);
		large = BoundingBox::MergeTwoBoundingBox(large, bound);
		box.Update(Conversion::fromVec3f(bound.m_min), 
				   Conversion::fromVec3f(bound.m_max));
		box.Draw();		
	}

	BVHTree tree(boundings, BVHTree::Middle);
	auto bvh = tree.GetBoundings();
	for (auto& b : bvh) {
		box.Update(Conversion::fromVec3f(b.m_min),
				   Conversion::fromVec3f(b.m_max));
		box.Draw();
	}
	box.Update(Conversion::fromVec3f(large.m_min),
			   Conversion::fromVec3f(large.m_max));
	box.Draw();
}
#endif

void RenderManager::PassSpecifiedListNormal(RenderList& list, std::function<Model* (const std::string&)> ObjGetter)
{
	ResourceManager& scene_manager = ResourceManager::getInstance();
	
	for (auto& [obj_name, mesh_name] : list) 
	{		
		RenderMesh*		mesh	= scene_manager.GetMeshByName(mesh_name);
		Model*			obj		= ObjGetter(obj_name);
		if (!obj || !mesh) continue;		
		auto material = obj->GetComponent<MaterialComponent>();		
		if (!material || material->GetShader() == "null") {
			continue;
		}
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
	rhi_device_->SetPolygonMode(RhiPolygonMode::Line);	
	for (auto& obj : planning_aux_lists_) {
		obj->SetRhiDevice(rhi_device_);
		obj->Draw(nullptr);
	}	
	rhi_device_->SetPolygonMode(RhiPolygonMode::Fill);
	
}

}