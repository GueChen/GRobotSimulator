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
#include "component/transform_component.h"

#ifdef _COLLISION_TEST
#include "component/collider_component.h"
#include "geometry/bounding_volume_hierarchy.h"
#endif

#include <QtGUI/QOpenGLContext>

#include <iostream>
#include <stdexcept>
#include <variant>

namespace GComponent {

namespace {

class RhiDebugGroupGuard {
public:
	RhiDebugGroupGuard(const shared_ptr<IRhiDevice>& rhi_device, std::string_view name)
		: rhi_device_(rhi_device)
	{
		if (rhi_device_) {
			rhi_device_->PushDebugGroup(name);
		}
	}

	~RhiDebugGroupGuard()
	{
		if (rhi_device_) {
			rhi_device_->PopDebugGroup();
		}
	}

private:
	shared_ptr<IRhiDevice> rhi_device_;
};

glm::vec3 GetMaterialVec3(MaterialComponent* material, std::initializer_list<std::string_view> names, const glm::vec3& fallback)
{
	if (!material) return fallback;
	for (auto& prop : material->GetProperties()) {
		for (auto name : names) {
			if (prop.name != name) continue;
			if (auto value = std::get_if<glm::vec3>(&prop.val)) {
				return *value;
			}
			if (auto value = std::get_if<Color>(&prop.val)) {
				return value->val;
			}
		}
	}
	return fallback;
}

float GetMaterialFloat(MaterialComponent* material, std::string_view name, float fallback)
{
	if (!material) return fallback;
	for (auto& prop : material->GetProperties()) {
		if (prop.name == name) {
			if (auto value = std::get_if<float>(&prop.val)) {
				return *value;
			}
		}
	}
	return fallback;
}

bool GetMaterialBool(MaterialComponent* material, std::string_view name, bool fallback)
{
	if (!material) return fallback;
	for (auto& prop : material->GetProperties()) {
		if (prop.name == name) {
			if (auto value = std::get_if<bool>(&prop.val)) {
				return *value;
			}
		}
	}
	return fallback;
}

bool SupportsDeferredFramebufferPasses(const shared_ptr<IRhiDevice>& rhi_device)
{
	return rhi_device && rhi_device->SupportsFeature(RhiDeviceFeature::MultipleColorAttachments);
}

bool HasRegisteredShader(ResourceManager& resource_manager, const std::string& shader_name)
{
	return resource_manager.GetShaderDescByName(shader_name) != nullptr;
}

} // namespace
	
/*__________________________PUBLIC METHODS____________________________________*/
RenderManager::~RenderManager() = default;

void RenderManager::InitFrameBuffer()
{	
	if (!rhi_device_) return;

	DestroyGBuffer();
	render_FBO_ = std::nullopt;
	selected_outline_FBO_ = std::nullopt;
	FrameBufferObject render_fbo(m_render_sharing_msg.viewport.window_size.x,
								 m_render_sharing_msg.viewport.window_size.y,
								 FrameBufferObject::Color,
								 rhi_device_);
	if (!render_fbo.IsAvailable()) {
		static bool logged_offscreen_skip = false;
		if (!logged_offscreen_skip) {
			std::cerr << "RenderManager offscreen passes skipped: primary render-target framebuffer creation failed\n";
			logged_offscreen_skip = true;
		}
		return;
	}
	render_FBO_ = std::move(render_fbo);

	FrameBufferObject outline_fbo(m_render_sharing_msg.viewport.window_size.x,
								  m_render_sharing_msg.viewport.window_size.y,
								  FrameBufferObject::Color,
								  rhi_device_);
	if (outline_fbo.IsAvailable()) {
		selected_outline_FBO_ = std::move(outline_fbo);
	}
	else {
		static bool logged_outline_skip = false;
		if (!logged_outline_skip) {
			std::cerr << "RenderManager outline mask pass skipped: secondary offscreen framebuffer creation failed\n";
			logged_outline_skip = true;
		}
	}
	InitGBuffer();
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

void RenderManager::SetRenderPipelineType(RenderPipelineType type)
{
	render_pipeline_type_ = type;
}

RenderManager::RenderPipelineType RenderManager::GetRenderPipelineType() const
{
	return render_pipeline_type_;
}

void RenderManager::SetRhiDevice(const shared_ptr<IRhiDevice>& rhi_device)
{
	rhi_device_ = rhi_device;

	InitFrameBuffer();
	depth_FBO_ = std::nullopt;
	FrameBufferObject depth_fbo =
#ifdef _USE_CSM
		FrameBufferObject(depth_buffer_resolustion_,
						  depth_buffer_resolustion_,
						  m_csm_levels,
						  FrameBufferObject::Depth,
						  rhi_device_);
#else	
		FrameBufferObject(depth_buffer_resolustion_, 
						  depth_buffer_resolustion_, 
						  FrameBufferObject::Depth,
						  rhi_device_);
#endif
	if (depth_fbo.IsAvailable()) {
		depth_FBO_ = std::move(depth_fbo);
	}
	else {
		static bool logged_depth_skip = false;
		if (!logged_depth_skip) {
			std::cerr << "RenderManager shadow depth pass skipped: shadow-map framebuffer creation failed\n";
			logged_depth_skip = true;
		}
	}

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
	RhiDebugGroupGuard frame_group(rhi_device_, render_pipeline_type_ == RenderPipelineType::Deferred ? "Frame - Deferred Pipeline" : "Frame - Forward Pipeline");
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
	if (depth_FBO_ && depth_FBO_->IsAvailable()) {
		rhi_device_->BindTextureUnit(3, RhiTextureHandle{ depth_FBO_->GetTextureID() });
	}
	else {
		rhi_device_->BindTextureUnit(3, {});
	}

	PickingPass();
	
	DepthMapPass();

	if (render_pipeline_type_ == RenderPipelineType::Deferred) {
		DeferredPass();
	}
	else {
		NormalPass();
	}
	
	
	PostProcessPass();

	Clear();
}

/*_____________________________PROTECTED METHODS__________________________________________*/
RenderManager::RenderManager() :grid_(50, 0.20f)
{}

/*_____________________________PRIVATE METHODS____________________________________________*/
void RenderManager::InitGBuffer()
{
	if (!rhi_device_) return;

	DestroyGBuffer();
	if (!SupportsDeferredFramebufferPasses(rhi_device_)) {
		static bool logged_gbuffer_skip = false;
		if (!logged_gbuffer_skip) {
			std::cerr << "Deferred GBuffer skipped: multiple color attachments are not supported by the active RHI backend\n";
			logged_gbuffer_skip = true;
		}
		return;
	}
	const int width = static_cast<int>(m_render_sharing_msg.viewport.window_size.x);
	const int height = static_cast<int>(m_render_sharing_msg.viewport.window_size.y);
	if (width <= 0 || height <= 0) return;

	gbuffer_.framebuffer = rhi_device_->CreateFramebuffer(RhiFramebufferCreateDesc{
		.width = width,
		.height = height,
		.color_attachments = {
			RhiTextureFormat::Rgba16Float,
			RhiTextureFormat::Rgba16Float,
			RhiTextureFormat::Rgba16Float,
			RhiTextureFormat::Rgba16Float
		}
	});
	for (uint32_t i = 0; i < gbuffer_.textures.size(); ++i) {
		gbuffer_.textures[i] = rhi_device_->GetFramebufferColorTexture(gbuffer_.framebuffer, i);
	}
	if (!gbuffer_.IsValid()) {
		static bool logged_gbuffer_creation_failure = false;
		if (!logged_gbuffer_creation_failure) {
			std::cerr << "Deferred GBuffer skipped: framebuffer attachments could not be created\n";
			logged_gbuffer_creation_failure = true;
		}
		DestroyGBuffer();
		return;
	}
	gbuffer_.width = width;
	gbuffer_.height = height;
}

void RenderManager::DestroyGBuffer()
{
	if (gbuffer_.framebuffer && rhi_device_) {
		rhi_device_->DestroyFramebuffer(gbuffer_.framebuffer);
	}
	gbuffer_ = {};
}

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
	IblSetupContext context;
	context.rhi_device = rhi_device_;
	context.matrices_ubo = matrices_UBO_ ? &matrices_UBO_.value() : nullptr;
	context.sky_box_mesh = resources.GetMeshByName(skybox_.getMesh());
	context.quad_mesh = resources.GetMeshByName(screen_quad_.getMesh());
	context.equirectangular_to_cube_shader = resources.GetShaderByName("equirectangular2cube");
	context.irradiance_shader = resources.GetShaderByName("irr_conv");
	context.prefilter_shader = resources.GetShaderByName("pft_conv");
	context.brdf_lut_shader = resources.GetShaderByName("brdf_lut");
	context.hdr_path = "./asset/textures/loft_newport/Newport_Loft_Ref.hdr";
	ibl_setup_result_ = SetupImageBasedLightingResources(context);
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
	if (!picking_controller_handle_ || !picking_controller_handle_->IsAvailable()) return;

	RhiDebugGroupGuard debug_group(rhi_device_, "Picking Pass");
	PickingGuard picking_guard(picking_controller_handle_.value());
	ClearGLScreenBuffer(0.0f, 0.0f, 0.0f, 1.0f);

	{
		RhiDebugGroupGuard scene_group(rhi_device_, "Picking Pass - Scene Objects");
		PassSpecifiedListPicking(PassType::DirLightPass, render_list_, [](const std::string& name) {
			return ModelManager::getInstance().GetModelByName(name);
		});
	}
		
	rhi_device_->Clear(RhiClearFlags::Depth);
	{
		RhiDebugGroupGuard auxiliary_group(rhi_device_, "Picking Pass - Auxiliary Objects");
		PassSpecifiedListPicking(PassType::AuxiliaryPass, post_process_list_, [](const std::string& name) {
			return ModelManager::getInstance().GetAuxiModelByName(name);
		});
	}
}

void RenderManager::DepthMapPass()
{				
	if (!depth_FBO_ || !depth_FBO_->IsAvailable()) {
		return;
	}

	RhiDebugGroupGuard debug_group(rhi_device_, "Shadow Depth Pass");
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
	if (!render_FBO_ || !render_FBO_->IsAvailable()) {
		return;
	}

	RhiDebugGroupGuard debug_group(rhi_device_, "Forward Render Pass");
	FBOGuard fbo_guard(&render_FBO_.value());

	ClearGLScreenBuffer(0.0f, 0.0f, 0.05f, 1.0f);
	
	RenderingPass();

	DrawSceneOverlays();
}

void RenderManager::DeferredPass()
{
	RhiDebugGroupGuard debug_group(rhi_device_, "Deferred Render Pass");
	static bool logged_deferred_fallback = false;
	if (!render_FBO_ || !render_FBO_->IsAvailable()) {
		if (!logged_deferred_fallback) {
			std::cerr << "Deferred pipeline fallback: primary offscreen framebuffer is not available\n";
			logged_deferred_fallback = true;
		}
		return;
	}
	if (!gbuffer_.IsValid() || !DeferredGeometryPass()) {
		if (!logged_deferred_fallback) {
			std::cerr << "Deferred pipeline fallback: GBuffer or geometry pass is not available\n";
			logged_deferred_fallback = true;
		}
		NormalPass();
		return;
	}

	bool lighting_pass_ok = false;
	{
		FBOGuard fbo_guard(&render_FBO_.value());
		ClearGLScreenBuffer(0.0f, 0.0f, 0.05f, 1.0f);
		lighting_pass_ok = DeferredLightingPass();
		if (lighting_pass_ok) {
			RhiDebugGroupGuard depth_group(rhi_device_, "Deferred Render Pass - Depth Restore");
			rhi_device_->Clear(RhiClearFlags::Depth);
			DeferredDepthPrepass();
			DrawSceneOverlays();
		}
	}

	if (!lighting_pass_ok) {
		if (!logged_deferred_fallback) {
			std::cerr << "Deferred pipeline fallback: lighting pass is not available\n";
			logged_deferred_fallback = true;
		}
		NormalPass();
		return;
	}
}

void RenderManager::DrawSceneOverlays()
{
	RhiDebugGroupGuard debug_group(rhi_device_, "Scene Overlay Pass");

	{
		RhiDebugGroupGuard simplex_group(rhi_device_, "Scene Overlay Pass - Simplex Meshes");
		SimplexMeshPass();
	}
	
#ifdef _COLLISION_TEST
	{
		RhiDebugGroupGuard collision_group(rhi_device_, "Scene Overlay Pass - Collision Debug");
		rhi_device_->SetPolygonMode(RhiPolygonMode::Line);
		rhi_device_->SetBlendAlpha();
		rhi_device_->Enable(RhiCapability::Blend);
		CollisionPass(render_list_, [](const std::string& name) {
			return ModelManager::getInstance().GetModelByName(name);
		});
		rhi_device_->Disable(RhiCapability::Blend);
		rhi_device_->SetPolygonMode(RhiPolygonMode::Fill);
	}
#define _DRAW_DBOUNDING_BOX
#ifdef _DRAW_DBOUNDING_BOX
	{
		RhiDebugGroupGuard bounds_group(rhi_device_, "Scene Overlay Pass - Bounding Boxes");
		BoundingBoxPass(render_list_, [](const std::string& name) {
			return ModelManager::getInstance().GetModelByName(name);
		});
	}
#endif
#endif

	// TODO: not so good try to hide it
	if (ibl_setup_result_.IsUsable()) {
		RhiDebugGroupGuard skybox_group(rhi_device_, "Scene Overlay Pass - Skybox");
		rhi_device_->SetDepthFunc(RhiDepthFunc::LessEqual);
		skybox_.Draw();
		rhi_device_->SetDepthFunc(RhiDepthFunc::Less);
	}
	else {
		static bool logged_skybox_skip = false;
		if (!logged_skybox_skip) {
			std::cerr << "Skybox pass skipped: IBL resources are " << ToString(ibl_setup_result_.status);
			if (!ibl_setup_result_.reason.empty()) {
				std::cerr << " (" << ibl_setup_result_.reason << ")";
			}
			std::cerr << '\n';
			logged_skybox_skip = true;
		}
	}

	{
		RhiDebugGroupGuard grid_group(rhi_device_, "Scene Overlay Pass - Grid");
		rhi_device_->Enable(RhiCapability::Blend);
		rhi_device_->SetBlendAlpha();
		grid_.SetRhiDevice(rhi_device_);
		grid_.Draw();
		rhi_device_->Disable(RhiCapability::Blend);
	}
	
}

void RenderManager::RenderingPass()
{
	RhiDebugGroupGuard debug_group(rhi_device_, "Forward Opaque Pass");
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

bool RenderManager::DeferredGeometryPass()
{
	RhiDebugGroupGuard debug_group(rhi_device_, "Deferred Geometry Pass - GBuffer");
	ResourceManager::getInstance().BindShader("deferred_geometry");
	MyShader* geometry_shader = ResourceManager::getInstance().GetShaderByName("deferred_geometry");
	if ((!geometry_shader && (ResourceManager::getInstance().GetActiveBackendType() == RhiBackendType::OpenGL
		|| !HasRegisteredShader(ResourceManager::getInstance(), "deferred_geometry"))) || !gbuffer_.IsValid()) {
		return false;
	}

	rhi_device_->BindFramebuffer(RhiFramebufferBindTarget::Framebuffer, gbuffer_.framebuffer);
	ClearGLScreenBuffer(0.0f, 0.0f, 0.0f, 1.0f);
	rhi_device_->Disable(RhiCapability::Blend);
	rhi_device_->Disable(RhiCapability::CullFace);
	if (geometry_shader) {
		geometry_shader->use();
		PassSpecifiedListDeferredGeometry(render_list_, [](const std::string& name) {
			return ModelManager::getInstance().GetModelByName(name);
		}, *geometry_shader);
	}
	else {
		PassSpecifiedListNormal(render_list_, [](const std::string& name) {
			return ModelManager::getInstance().GetModelByName(name);
		});
	}
	rhi_device_->BindDefaultFramebuffer(RhiFramebufferBindTarget::Framebuffer);
	return true;
}

bool RenderManager::DeferredLightingPass()
{
	RhiDebugGroupGuard debug_group(rhi_device_, "Deferred Lighting Pass");
	ResourceManager::getInstance().BindShader("deferred_lighting");
	MyShader* lighting_shader = ResourceManager::getInstance().GetShaderByName("deferred_lighting");
	RenderMesh* quad_mesh = ResourceManager::getInstance().GetMeshByName("quads");
	if ((!lighting_shader && (ResourceManager::getInstance().GetActiveBackendType() == RhiBackendType::OpenGL
		|| !HasRegisteredShader(ResourceManager::getInstance(), "deferred_lighting"))) || !quad_mesh || !gbuffer_.IsValid()) {
		return false;
	}

	for (uint32_t i = 0; i < gbuffer_.textures.size(); ++i) {
		rhi_device_->BindTextureUnit(10 + i, gbuffer_.textures[i]);
	}

	rhi_device_->Disable(RhiCapability::Blend);
	rhi_device_->Disable(RhiCapability::CullFace);
	rhi_device_->SetDepthFunc(RhiDepthFunc::LessEqual);
	if (lighting_shader) {
		lighting_shader->use();
	}
	quad_mesh->Draw();
	rhi_device_->SetDepthFunc(RhiDepthFunc::Less);
	for (uint32_t i = 0; i < gbuffer_.textures.size(); ++i) {
		rhi_device_->BindTextureUnit(10 + i, {});
	}
	return true;
}

void RenderManager::DeferredDepthPrepass()
{
	RhiDebugGroupGuard debug_group(rhi_device_, "Deferred Depth Prepass");
	ResourceManager::getInstance().BindShader("deferred_depth");
	MyShader* depth_shader = ResourceManager::getInstance().GetShaderByName("deferred_depth");
	if (!depth_shader && (ResourceManager::getInstance().GetActiveBackendType() == RhiBackendType::OpenGL
		|| !HasRegisteredShader(ResourceManager::getInstance(), "deferred_depth"))) return;

	rhi_device_->Disable(RhiCapability::Blend);
	if (depth_shader) {
		depth_shader->use();
		PassSpecifiedListDeferredDepth(render_list_, [](const std::string& name) {
			return ModelManager::getInstance().GetModelByName(name);
		}, *depth_shader);
	}
	else {
		PassSpecifiedListNormal(render_list_, [](const std::string& name) {
			return ModelManager::getInstance().GetModelByName(name);
		});
	}
}

void RenderManager::SelectedOutlinePass()
{
	if (!selected_outline_FBO_ || !selected_outline_FBO_->IsAvailable()) return;

	RhiDebugGroupGuard debug_group(rhi_device_, "Selected Outline Mask Pass");
	FBOGuard outline_guard(&selected_outline_FBO_.value());
	ClearGLScreenBuffer(0.0f, 0.0f, 0.0f, 1.0f);

	if (m_selected_id == 0) return;

	Model* selected_obj = ModelManager::getInstance().GetModelByHandle(m_selected_id);
	if (!selected_obj) return;

	RenderMesh* mesh = ResourceManager::getInstance().GetMeshByName(selected_obj->getMesh());
	ResourceManager::getInstance().BindShader("outline");
	MyShader* outline_shader = ResourceManager::getInstance().GetShaderByName("outline");
	if (!mesh || (!outline_shader && (ResourceManager::getInstance().GetActiveBackendType() == RhiBackendType::OpenGL
		|| !HasRegisteredShader(ResourceManager::getInstance(), "outline")))) return;

	auto* transform = selected_obj->GetComponent<TransformComponent>();
	if (!transform) return;

	const glm::mat4 model = Conversion::fromMat4f(transform->GetModelGlobal());

	if (outline_shader) {
		outline_shader->use();
		outline_shader->setMat4("model", model);
	}
	rhi_device_->Disable(RhiCapability::CullFace);
	mesh->Draw();
}

void RenderManager::PostProcessPass()
{
	if (!render_FBO_ || !render_FBO_->IsAvailable()) {
		return;
	}

	RhiDebugGroupGuard debug_group(rhi_device_, "Post Process Pass");
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
	
	{
		RhiDebugGroupGuard auxiliary_front_group(rhi_device_, "Post Process Pass - Auxiliary Front Faces");
		rhi_device_->Enable(RhiCapability::Blend);
		rhi_device_->SetCullFace(RhiCullFace::Front);
		rhi_device_->SetBlendAlpha();
		PassSpecifiedListNormal(post_process_list_, [](const std::string& name) {
			return ModelManager::getInstance().GetAuxiModelByName(name);
		});
	}

	{
		RhiDebugGroupGuard auxiliary_back_group(rhi_device_, "Post Process Pass - Auxiliary Back Faces");
		rhi_device_->SetCullFace(RhiCullFace::Back);
		PassSpecifiedListNormal(post_process_list_, [](const std::string& name) {
			return ModelManager::getInstance().GetAuxiModelByName(name);
		});
	}
	rhi_device_->Disable(RhiCapability::Blend);
}

void RenderManager::PassSpecifiedListPicking(PassType draw_index_type, RenderList& list, function<Model* (const std::string&)> ObjGetter)
{
	ResourceManager& scene_manager	= ResourceManager::getInstance();
	ModelManager&	 model_manager	= ModelManager::getInstance();

	// Universal Shader Uniform Attribute Settings
	scene_manager.BindShader("picking");
	MyShader*		 picking_shader = scene_manager.GetShaderByName("picking");
	if (!picking_shader && (scene_manager.GetActiveBackendType() == RhiBackendType::OpenGL || !HasRegisteredShader(scene_manager, "picking"))) {
		static bool logged_missing_picking_shader = false;
		if (!logged_missing_picking_shader) {
			std::cerr << "Picking pass skipped: picking shader is not available\n";
			logged_missing_picking_shader = true;
		}
		return;
	}
	if (picking_shader) {
		picking_shader->use();
	}

	if (picking_shader) {
		picking_shader->setUint("gDrawIndex", static_cast<unsigned>(draw_index_type));
	}
	
	//  Pass Normally
	for (auto& [obj_name, mesh_name] : list) 
	{		
		RenderMesh*	mesh  = scene_manager.GetMeshByName(mesh_name);
		Model*		obj	  = ObjGetter(obj_name);
		auto&		trans = *obj->GetComponent<TransformComponent>();
		if (picking_shader) {
			picking_shader->setUint("gModelIndex", obj->model_id_);
			picking_shader->setMat4("model",	   Conversion::fromMat4f(trans.GetModelGlobal()));
		}
		
		if (mesh) mesh->Draw();
	}
}

void RenderManager::PassSpecifiedListDepth(RenderList& list, function<Model* (const std::string&)> ObjGetter)
{
	ResourceManager& scene_manager = ResourceManager::getInstance();
	ModelManager&	 model_manager = ModelManager::getInstance();

#ifdef _USE_CSM
	const char* depth_shader_name = "csm_depth_map";
#else 
	const char* depth_shader_name = "depth_map";
#endif
	scene_manager.BindShader(depth_shader_name);
	MyShader* depth_shader = scene_manager.GetShaderByName(depth_shader_name);
	if (!depth_shader && (scene_manager.GetActiveBackendType() == RhiBackendType::OpenGL || !HasRegisteredShader(scene_manager, depth_shader_name))) {
		static bool logged_missing_depth_shader = false;
		if (!logged_missing_depth_shader) {
			std::cerr << "Depth pass skipped: depth shader is not available\n";
			logged_missing_depth_shader = true;
		}
		return;
	}
	if (depth_shader) {
		depth_shader->use();
	}

	for (auto& [obj_name, mesh_name] : list) 
	{		
		RenderMesh* mesh = scene_manager.GetMeshByName(mesh_name);
		Model* obj = ObjGetter(obj_name);
		auto& trans = *obj->GetComponent<TransformComponent>();
		if (depth_shader) {
			depth_shader->setMat4("model", Conversion::fromMat4f(trans.GetModelGlobal()));
		}

		if (mesh) mesh->Draw();
	}
}

#ifdef _COLLISION_TEST
void RenderManager::CollisionPass(RenderList&list, function<RawptrModel(const std::string&)> ObjGetter)
{
	ResourceManager& scene_manager = ResourceManager::getInstance();
	MyShader*		 base_shader   = scene_manager.GetShaderByName("base");
	scene_manager.BindShader("base");
	if (!base_shader && (scene_manager.GetActiveBackendType() == RhiBackendType::OpenGL || !HasRegisteredShader(scene_manager, "base"))) {
		static bool logged_missing_base_shader = false;
		if (!logged_missing_base_shader) {
			std::cerr << "Collision pass skipped: base shader is not available\n";
			logged_missing_base_shader = true;
		}
		return;
	}
	if (base_shader) {
		base_shader->use();
	}
	
	for (auto& [obj_name, mesh_name] : list) {
		RenderMesh* mesh = scene_manager.GetMeshByName(mesh_name);
		Model*      obj  = ObjGetter(obj_name);
		if (!obj || !mesh) continue;
		auto& trans = *obj->GetComponent<TransformComponent>();
		if (base_shader) {
			base_shader->setMat4("model", Conversion::fromMat4f(trans.GetModelGlobal()));
		}
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

void RenderManager::PassSpecifiedListDeferredGeometry(RenderList& list, function<RawptrModel(const std::string&)> ObjGetter, MyShader& shader)
{
	ResourceManager& scene_manager = ResourceManager::getInstance();

	for (auto& [obj_name, mesh_name] : list)
	{
		RenderMesh* mesh = scene_manager.GetMeshByName(mesh_name);
		Model* obj = ObjGetter(obj_name);
		if (!obj || !mesh) continue;

		auto* transform = obj->GetComponent<TransformComponent>();
		auto* material = obj->GetComponent<MaterialComponent>();
		if (!transform || !material || material->GetShader() == "axis" || material->GetShader() == "postprocess") {
			continue;
		}

		const glm::vec3 albedo = GetMaterialVec3(material, { "albedo color", "color" }, glm::vec3(1.0f));
		const float metallic = GetMaterialFloat(material, "metallic", 0.0f);
		const float roughness = GetMaterialFloat(material, "roughness", 0.5f);
		const float ao = GetMaterialFloat(material, "ao", 1.0f);
		const bool accept_shadow = GetMaterialBool(material, "accept shadow", material->GetIsCastShadow());

		shader.setMat4("model", Conversion::fromMat4f(transform->GetModelGlobal()));
		shader.setVec3("albedo_color", albedo);
		shader.setFloat("metallic", metallic);
		shader.setFloat("roughness", roughness);
		shader.setFloat("ao", ao);
		shader.setBool("accept_shadow", accept_shadow);
		mesh->Draw();
	}
}

void RenderManager::PassSpecifiedListDeferredDepth(RenderList& list, function<RawptrModel(const std::string&)> ObjGetter, MyShader& shader)
{
	ResourceManager& scene_manager = ResourceManager::getInstance();

	for (auto& [obj_name, mesh_name] : list)
	{
		RenderMesh* mesh = scene_manager.GetMeshByName(mesh_name);
		Model* obj = ObjGetter(obj_name);
		if (!obj || !mesh) continue;

		auto* transform = obj->GetComponent<TransformComponent>();
		if (!transform) continue;

		shader.setMat4("model", Conversion::fromMat4f(transform->GetModelGlobal()));
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