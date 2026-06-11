/**
 *  @file  	rendermanager.h
 *  @brief 	This Class is responsible for all draw call and shader properties settings.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	May 7, 2022
 **/
#ifndef _RENDERMANAGER_H
#define _RENDERMANAGER_H

#include "base/singleton.h"

#include "render/rhi/render_global_info.h"
#include "render/rendermesh.h"
#include "render/framebufferobject.h"
#include "render/uniform_buffer_object.h"
#include "render/rhi/ibl_resource_setup.h"
#include "render/rhi/rhi_device.h"
#include "function/picking_helper.h"
#include "model/basegrid.h"
#include "model/basic/postprocess_quads.h"
#include "model/basic/skybox.h"

#include "simplexmesh/basicmesh"

#include <QtCore/QObject>
#include <QtOpenGL/QOpenGLFramebufferObjectFormat>
#include <glm/glm.hpp>

#include <functional>
#include <optional>
#include <thread>
#include <memory>
#include <string>
#include <array>

namespace GComponent {

class Model;
class PickingController;
class MyShader;

using std::list;
using std::string;
using std::function;
using std::shared_ptr;
using std::optional;

enum class PassType {
	DirLightPass  = 1,
	AuxiliaryPass = 2
};

struct RenderCommand {
	string object_name;	
	string mesh_name;	
};

class RenderManager : public SingletonBase<RenderManager> 
{
/// type alias
	using RenderList  = list<RenderCommand>;
	using RawptrModel = Model*;

/// friends and macro
	friend class SingletonBase<RenderManager>;
	NonCopyable(RenderManager)

public:
	enum QueueType {
		Normal, PostProcess, Depth
	};

	enum class RenderPipelineType {
		Forward,
		Deferred
	};

/*_________Public Inteface Methods___________________________________________________________*/
public:
	virtual ~RenderManager();
	
	void tick();
	
	void SetRhiDevice(const shared_ptr<IRhiDevice>& rhi_device);

	void SetPickingController(PickingController& controller);

	void SetRenderPipelineType(RenderPipelineType type);
	[[nodiscard]] RenderPipelineType GetRenderPipelineType() const;

	void InitFrameBuffer();

//________________Render relate Invoke inteface______________________________________________//
	void EmplaceRenderCommand	  (std::string obj_name, std::string mesh_name, QueueType type);
	void EmplaceFrontRenderCommand(std::string obj_name, std::string mesh_name, QueueType type);
//_______________Planing Relate Auxiliry Inteface_______________________________________________//
	void EmplaceAuxiliaryObj(shared_ptr<SimplexModel>&& obj);
	void ClearAuxiliaryObj();

protected:
	RenderManager();

private:	
/*_______________________Rendering Datas Setting___________________________________________*/
	void SetProjectViewMatrices();
	void SetDirLightViewPosition();
/*_______________________Physics Based Rendering and Image Based Lighting__________________*/
	void InitializeIBLResource();
	
/*_______________________Cascade Shadow Map Helper Methods_________________________________*/
	std::vector<glm::vec4>	GetFrustumCornersWorldSpace(const glm::mat4& projection, const glm::mat4& view);
	glm::mat4				GetLightViewProjMatrix(const float near_plane, const float far_plane);
	std::vector<glm::mat4>	GetLightViewProjMatrices();

/*_______________________Rendering Passsing________________________________________________*/
	void Clear();
	void ClearGLScreenBuffer(float r, float g, float b, float a);
	void ClearList();

	void PickingPass();
	void DepthMapPass();
	void NormalPass();
	void DeferredPass();
	void PostProcessPass();
	void RenderingPass();
	void DrawSceneOverlays();
	void SelectedOutlinePass();
	bool DeferredGeometryPass();
	bool DeferredLightingPass();
	void DeferredDepthPrepass();
		
	void PassSpecifiedListPicking	(PassType draw_index_type, 
									 RenderList&,	 function<RawptrModel(const std::string&)>ObjGetter);
	void PassSpecifiedListNormal	(RenderList&,	 function<RawptrModel(const std::string&)>ObjGetter);
	void PassSpecifiedListDepth		(RenderList&,	 function<RawptrModel(const std::string&)>ObjGetter);			
	void PassSpecifiedListDeferredGeometry(RenderList&, function<RawptrModel(const std::string&)>ObjGetter, MyShader& shader);
	void PassSpecifiedListDeferredDepth(RenderList&, function<RawptrModel(const std::string&)>ObjGetter, MyShader& shader);

#ifdef _COLLISION_TEST
	void CollisionPass  (RenderList&, function<RawptrModel(const std::string&)>ObjGetter);
	void BoundingBoxPass(RenderList&, function<RawptrModel(const std::string&)>ObjGetter);
#endif


/*_______________________Planning Simplex Render_____________________________________________*/
	void SimplexMeshPass();

/*________________________FIELDS_____________________________________________________________*/
public:
	RenderGlobalInfo				m_render_sharing_msg;
	unsigned						m_selected_id				= 0;
	
/*______________________Cascade Shadow Map____________________________________________________*/
	std::vector<float>				m_csm_cascade_planes		= {};
	unsigned						m_csm_levels				= 5;

private:
	struct GBuffer {
		RhiFramebufferHandle framebuffer;
		std::array<RhiTextureHandle, 4> textures = {};
		int width = 0;
		int height = 0;

		[[nodiscard]] bool IsValid() const { return framebuffer.IsValid() && textures[0].IsValid(); }
	};

	void InitGBuffer();
	void DestroyGBuffer();

	list<RenderCommand>				render_list_;
	list<RenderCommand>				post_process_list_;
	list<RenderCommand>				shadow_cast_list_;

	optional<PickingController>		picking_controller_handle_;
	shared_ptr<IRhiDevice>			rhi_device_;
	RenderPipelineType				render_pipeline_type_		= RenderPipelineType::Forward;
	GBuffer							gbuffer_;
/*_______________________Bad Practice Modified it In a Better Place___________________________*/
	BaseGrid						grid_;
	SkyBox							skybox_;
	PostprocessQuads				screen_quad_;	
	optional<FrameBufferObject>		render_FBO_					= std::nullopt;
	optional<FrameBufferObject>		selected_outline_FBO_		= std::nullopt;

	optional<FrameBufferObject>     depth_FBO_					= std::nullopt;
	const int                       depth_buffer_resolustion_   = 4096;
	
	optional<UniformBufferObject>   matrices_UBO_				= std::nullopt;
	optional<UniformBufferObject>   light_matrices_UBO_			= std::nullopt;
	optional<UniformBufferObject>	ambient_observer_UBO_		= std::nullopt;	
	IblSetupResult					ibl_setup_result_{};
	
/*________________________Planning Display Related_____________________________________________*/
	list<std::shared_ptr<SimplexModel>> 
									planning_aux_lists_			= {};
	int								delete_count_				= 0;
	std::mutex						planning_lock_;	
};
}

#endif // !_RENDERMANAGER_H
