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
#include "render/myshader.h"
#include "render/mygl.hpp"
#include "function/picking_helper.h"

#include <QtCore/QObject>

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

	void PushRenderCommand(const RenderCommand & command);
	void EmplaceRenderCommand(string obj_name, string shader_name, 
							  string mesh_name, string uniform_name = "");
	
	void PushAuxiRenderCommand(const RenderCommand& command);
	void EmplaceAuxiRenderCommand(
							  string obj_name,  string shader_name,
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
	void RenderingPass();
	void AuxiliaryPass();
	void PostProcessPass();

	void PassSpecifiedListPicking(PassType draw_index_type, list<RenderCommand>&, function<Model* (const std::string&)>ObjGetter);
	void PassSpecifiedListNormal(list<RenderCommand>&,  function<Model*(const std::string&)>ObjGetter);

private:
	list<RenderCommand>				axui_render_list_;
	list<RenderCommand>				render_list_;
	optional<PickingController>		picking_controller_handle_;
	shared_ptr<MyGL>				gl_;

private:
class DisableGuard {
	public:
		DisableGuard(MyGL * gl, unsigned short command): gl_(gl), command_(command) 
		{ gl_->glDisable(command_); }
		~DisableGuard()
		{ gl_->glEnable(command_);  }
	private:
		MyGL* gl_;
		unsigned short command_;
	};
};
}

#endif // !_RENDERMANAGER_H
