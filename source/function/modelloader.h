#ifndef MODELLOADER_H
#define MODELLOADER_H
#include <string>
#include "component/mesh_component.h"

#define cPathModel(name)("./modifyCoordinate/" name)
#define PathModel(name) ("./modifyCoordinate/"#name)
#define sPathModel(name) ("./modifyCoordinate/" + name)

namespace GComponent {
    
class ModelLoader
{
public:
    ModelLoader() = delete;
    static std::tuple<std::vector<Vertex>, std::vector<Triangle>> readFile(const std::string & filePath);
    static std::tuple<std::vector<Vertex>, std::vector<Triangle>> readPlyFile(const std::string & filePath);
    static std::tuple<std::vector<Vertex>, std::vector<Triangle>> readSTLFile(const std::string& filePath);
    static std::string getFileContent(const std::string& filePath);

    static std::vector<float> vStringToFloat(const std::vector<std::string> & vs);
    static MeshComponent getMesh(const std::string & resource);
    static MeshComponent* getMeshPtr(const std::string& resource);
};

}


#endif // MODELLOADER_H
