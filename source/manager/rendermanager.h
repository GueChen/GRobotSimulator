/**
 *  @file  	rendermanager.h
 *  @brief 	This Class is responsible for all draw call and shader properties settings.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	May 7, 2022
 **/
#ifndef _RENDERMANAGER_H
#define _RENDERMANAGER_H

#include "base/singleton.h"

#include "render/rendermesh.h"
#include "render/framebufferobject.h"
#include "render/myshader.h"
#include "render/mygl.hpp"
#include "function/picking_helper.h"
#include "model/basegrid.h"
#include "model/basic/postprocess_quads.h"
#include "model/basic/skybox.h"

#include <QtCore/QObject>
#include <QtOpenGL/QOpenGLFramebufferObjectFormat>
#include <glm/glm.hpp>

#include <functional>
#include <optional>
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
	string shader_name;
	string mesh_name;
	string uniform_name;
};

class RenderManager : public SingletonBase<RenderManager>, public QObject 
{
/// type alias
	using RenderList  = list<RenderCommand>;
	using RawptrModel = Model*;

/// friends and macro
	friend class SingletonBase<RenderManager>;
	NonCoyable(RenderManager)

/*_________Public Inteface Methods___________________________________________________________*/
public:
	virtual ~RenderManager();
	
	void tick();
	
	void SetGL(const shared_ptr<MyGL>& gl);

	void SetPickingController(PickingController& controller);

	bool InitFrameBuffer();

//________________Render relate Invoke inteface______________________________________________//
	void PushRenderCommand(const RenderCommand & command);
	void EmplaceRenderCommand(string obj_name, string shader_name, 
							  string mesh_name, string uniform_name = "");
	
	void PushAuxiRenderCommand(const RenderCommand& command);
	void EmplaceAuxiRenderCommand(
							  string obj_name,  string shader_name,
							  string mesh_name, string uniform_name = "");

	void PushPostProcessRenderCommand(const RenderCommand& command);
	void EmplacePostProcessRenderCommand(
							  string obj_name,  string shader_name,
							  string mesh_name, string uniform_name = "");
	void EmplaceFrontPostProcessRenderCommand(
							  string obj_name,	string shader_name,
							  string mesh_name, string uniform_name = "");

protected:
	RenderManager();

private:	
/*_______________________Rendering Datas Setting___________________________________________*/
	void SetProjectViewMatrices();
	void SetDirLightViewPosition();

	std::vector<glm::vec4> GetFrustumCornersWorldSpace(const glm::mat4& projection, const glm::mat4& view);
	glm::mat4 GetLightViewProjMatrix(const float near_plane, const float far_plane);
	std::vector<glm::mat4> GetLightViewProjMatrices();

/*_______________________Rendering Passsing________________________________________________*/
	void Clear();
	void ClearGLScreenBuffer(float r, float g, float b, float a);
	void ClearList();

	void PickingPass();
	void DepthMapPass();
	void NormalPass();
	void PostProcessPass();
	void RenderingPass();
	void AuxiliaryPass();
		
	void PassSpecifiedListPicking(PassType draw_index_type, 
								 RenderList&,	 function<RawptrModel(const std::string&)>ObjGetter);
	void PassSpecifiedListNormal(RenderList&,	 function<RawptrModel(const std::string&)>ObjGetter);
	void PassSpecifiedListDepth(RenderList&,	 function<RawptrModel(const std::string&)>ObjGetter);	
	void PassSpecifiedListShadow(RenderList&,	 function<RawptrModel(const std::string&)>ObjGetter);
	void PassSpecifiedListCSMDepth(RenderList&,  function<RawptrModel(const std::string&)>ObjGetter);
	void PassSpecifiedListCSMShadow(RenderList&, function<RawptrModel(const std::string&)>ObjGetter);
/*________________________FIELDS_____________________________________________________________*/
public:
	unsigned						m_width						= 0;
	unsigned						m_height					= 0;
	
	float							m_camera_near_plane			= 0.1f;
	float							m_camera_far_plane			= 1000.0f;
	std::vector<float>				m_csm_cascade_planes		= {};

	// try to remove this part after consideration over the design
	glm::mat4						m_projction_mat				= glm::mat4(1.0f);
	glm::mat4						m_view_mat					= glm::mat4(1.0f);
	glm::vec3						m_light_dir					= glm::vec3(0.0f);
	glm::vec3						m_light_color				= glm::vec3(1.0f);
	glm::vec3						m_view_pos					= glm::vec3(0.0f);
	float							m_camera_zoom				= 0.0f;
	float							m_aspect					= 0.0f;

	unsigned						m_csm_matrices_UBO			= 0;
	unsigned						m_csm_depth_FBO				= 0;
	unsigned						m_csm_depth_texture			= 0;
	unsigned						m_csm_levels				= 5;
	unsigned						m_csm_texture_resolusion	= 4096;

private:
	list<RenderCommand>				axui_render_list_;
	list<RenderCommand>				render_list_;
	list<RenderCommand>				post_process_list_;

	optional<PickingController>		picking_controller_handle_;
	shared_ptr<MyGL>				gl_;

	BaseGrid						grid_;
	SkyBox							skybox_;
	PostprocessQuads				screen_quad_;	
	optional<FrameBufferObject>		FBO_						= std::nullopt;

	unsigned						depth_FBO_					= 0;
	unsigned						depth_texture_				= 0;

	size_t                          matrices_UBO_				= 0;
	size_t                          ambient_observer_UBO_		= 0;

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
