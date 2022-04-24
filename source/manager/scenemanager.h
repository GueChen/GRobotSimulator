#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include "base/singleton.h"

#include <unordered_map>
#include <string>
#include <memory>

namespace GComponent {
/// Ç°ÖÃÉùÃ÷ Forward Declaration
class MyShader;
class Mesh;
class MyGL;

using std::string;
using std::unique_ptr;
using std::shared_ptr;
using std::unordered_map;

class SceneManager : public SingletonBase<SceneManager>
{
    friend class SingletonBase<SceneManager>;
    NonCoyable(SceneManager)
public:

    void RegisteredMesh(const string& name, Mesh* raw_ptr_mesh);
    void DeregisteredMesh(const string& name);
    Mesh* GetMeshByName(const string& name);

    void RegisteredShader(const string& name, MyShader* raw_ptr_shader);
    void DeregisteredShader(const string& name);
    MyShader* GetShaderByName(const string& name);
    
    void SetGL(const shared_ptr<MyGL>& gl);

protected:
    SceneManager() = default;

private:
    unordered_map <string, unique_ptr<Mesh>>     mesh_map_;
    unordered_map <string, unique_ptr<MyShader>> shader_map_;

};

}
#endif // SCENEMANAGER_H
