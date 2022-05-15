#ifndef _MODELLOADER_QGLADAPTER_H
#define _MODELLOADER_QGLADAPTER_H

#include "function/modelloader.h"
#include "component/mesh_component.h"

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
	static MeshComponent  getMesh(const std::string& resource);
	static MeshComponent* getMeshPtr(const std::string& resource);
};
}
}


#endif // !_MODELLOADER_QGLADAPTER_H



