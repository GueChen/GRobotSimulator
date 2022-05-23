#ifndef _MODELLOADER_QGLADAPTER_H
#define _MODELLOADER_QGLADAPTER_H

#include "function/modelloader.h"
#include "render/rendermesh.h"

#include <string>

#define cPathModel(name)("./modifyCoordinate/" name)
#define PathModel(name) ("./modifyCoordinate/"#name)
#define sPathModel(name) ("./modifyCoordinate/" + name)

namespace GComponent {
namespace QGL {
class ModelLoader
{
public:
	ModelLoader() = delete;
	static RenderMesh  getMesh(const std::string& resource);
	static RenderMesh* getMeshPtr(const std::string& resource);
};
}
}


#endif // !_MODELLOADER_QGLADAPTER_H



