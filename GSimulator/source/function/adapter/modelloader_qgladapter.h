#ifndef _MODELLOADER_QGLADAPTER_H
#define _MODELLOADER_QGLADAPTER_H

#include "function/modelloader.h"
#include "render/rendermesh.h"

#include <string>

#define cPathModel(name)("./asset/stls/" name)
#define PathModel(name) ("./asset/stls/"#name)
#define sPathModel(name) ("./asset/stls/" + name)

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



