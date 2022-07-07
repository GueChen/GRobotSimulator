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
	friend class SingletonBase<RenderManager>;
	NonCoyable(RenderManager)
public:
	virtual ~RenderManager();
	
	bool InitFrameBuffer();

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

	void SetPickingController(PickingController& controller);

	void SetGL(const shared_ptr<MyGL>& gl);
	void tick();
protected:
	RenderManager();

private:
	void Clear();
	void ClearGLScreenBuffer(float r, float g, float b, float a);
	void ClearList();
	
	void PickingPass();
	void ShadowPass();
	void NormalPass();
	void PostProcessPass();
	void RenderingPass();
	void AuxiliaryPass();
	

	void PassSpecifiedListPicking(PassType draw_index_type, list<RenderCommand>&, function<Model* (const std::string&)>ObjGetter);
	void PassSpecifiedListNormal(list<RenderCommand>&,  function<Model*(const std::string&)>ObjGetter);

public:
	unsigned						m_width						= 0;
	unsigned						m_height					= 0;

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
