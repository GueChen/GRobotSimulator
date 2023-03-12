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
#include "render/mygl.hpp"
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

namespace GComponent {

class Model;
class PickingController;

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

/*_________Public Inteface Methods___________________________________________________________*/
public:
	virtual ~RenderManager();
	
	void tick();
	
	void SetGL(const shared_ptr<MyGL>& gl);

	void SetPickingController(PickingController& controller);

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
	void PostProcessPass();
	void RenderingPass();
		
	void PassSpecifiedListPicking	(PassType draw_index_type, 
									 RenderList&,	 function<RawptrModel(const std::string&)>ObjGetter);
	void PassSpecifiedListNormal	(RenderList&,	 function<RawptrModel(const std::string&)>ObjGetter);
	void PassSpecifiedListDepth		(RenderList&,	 function<RawptrModel(const std::string&)>ObjGetter);	
	void PassSpecifiedListShadow	(RenderList&,	 function<RawptrModel(const std::string&)>ObjGetter);
	void PassSpecifiedListCSMDepth	(RenderList&,    function<RawptrModel(const std::string&)>ObjGetter);

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
	list<RenderCommand>				render_list_;
	list<RenderCommand>				post_process_list_;
	list<RenderCommand>				shadow_cast_list_;

	optional<PickingController>		picking_controller_handle_;
	shared_ptr<MyGL>				gl_;
/*_______________________Bad Practice Modified it In a Better Place___________________________*/
	BaseGrid						grid_;
	SkyBox							skybox_;
	PostprocessQuads				screen_quad_;	
	optional<FrameBufferObject>		render_FBO_					= std::nullopt;

	optional<FrameBufferObject>     depth_FBO_					= std::nullopt;
	const int                       depth_buffer_resolustion_   = 4096;
	
	optional<UniformBufferObject>   matrices_UBO_				= std::nullopt;
	optional<UniformBufferObject>   light_matrices_UBO_			= std::nullopt;
	optional<UniformBufferObject>	ambient_observer_UBO_		= std::nullopt;

/*________________________Planning Display Related_____________________________________________*/
	list<std::shared_ptr<SimplexModel>> 
									planning_aux_lists_			= {};
	int								delete_count_				= 0;
	std::mutex						planning_lock_;	
/*________________________PROXY CLASS_______________________________________________________*/
private:
class DisableGuard {
	public:
		DisableGuard(MyGL * gl, unsigned short command): gl_(gl), command_(command) 
		{ gl_->glDisable(command_); }
		~DisableGuard()
		{ gl_->glEnable(command_);  }
	private:
		MyGL* gl_					= nullptr;
		unsigned short command_		= 0;
};
};
}

#endif // !_RENDERMANAGER_H
