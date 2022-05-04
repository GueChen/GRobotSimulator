#ifndef _RENDERMANAGER_H
#define _RENDERMANAGER_H

#include "base/singleton.h"

#include "render/myshader.h"
#include "render/mygl.hpp"
#include "component/mesh_component.h"

#include <QtCore/QObject>

#include <memory>
#include <string>
#include <variant>


namespace GComponent {

using std::string;
using std::shared_ptr;

struct ShaderBind {
	std::list<std::pair<std::string, std::variant<bool, unsigned, int, float, double>>> m_bind_list;
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
	
	void SetGL(const shared_ptr<MyGL>& gl);
	void tick();
protected:
	RenderManager();

private:
	void ClearGL();

	void PickingPass();
	void RenderingPass();
	void AuxiliaryPass();
	void PostProcessPass();


private:
	std::list<RenderCommand> render_list_;

	shared_ptr<MyGL>		 gl_;
};
}

#endif // !_RENDERMANAGER_H
