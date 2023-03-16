/**
 *  @file  	resourcemanager.h
 *  @brief 	The Resource Manager by this Class include shader, mesh Resources.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Apri 25, 2022
 **/
#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include "base/singleton.h"

#include "render/rendermesh.h"
#include "render/myshader.h"
#include "render/camera.hpp"
#include "render/mygl.hpp"

#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QtCore/QObject>
#include <QtCore/QTimer>

#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <list>

namespace GComponent {

/// 前置声明 Forward Declaration
class MyGL;

using std::list;
using std::string;
using std::string_view;
using std::vector;
using std::unique_ptr;
using std::shared_ptr;
using std::unordered_map;

struct TextureMsg {
    string                  name;
    string_view             path;
    string                  type;
    unsigned*               handle = nullptr;
};

struct CubemapMsg {
    string                  name;
    vector<string_view>     paths;
    string                  type;
    unsigned*               handle = nullptr;
};

class ResourceManager : public QObject
{
    Q_OBJECT

    NonCopyable(ResourceManager)

public:
    static ResourceManager& getInstance();
    ~ResourceManager();

    void EnablePickingMode();

    void            RegisteredMesh(const string& name, RenderMesh* raw_ptr_mesh);
    inline void     DeregisteredMesh(const string& name)    { DeregisteredSpecificMapElement(mesh_map_, name); }
    RenderMesh*     GetMeshByName(const string& name);

    void            RegisteredShader(const string& name, MyShader* raw_ptr_shader);
    inline void     DeregisteredShader(const string& name)  { DeregisteredSpecificMapElement(shader_map_, name); }
    MyShader*       GetShaderByName(const string& name);
    std::vector<std::string>
                    GetShadersName() const;

    void            RegisteredUIHandle(const string& name, QOpenGLWidget* ui_handle);
    void            DeregisteredUIHandle(const string& name);
    QOpenGLWidget*  GetUISurfaceByName(const string& name);
    
    // TODO: reconstructor texture structure
    void            RegisteredTexture(const TextureMsg& msg);
    void            RegisteredCubemap(const CubemapMsg& msg);
    void            DeregisteredTexture(const string& name);
    void            RegisteredTexture(const std::string& name, unsigned int tex);
    Texture         GetTextureByName(const string& name);


    void            SetGL(const shared_ptr<MyGL>& gl);

    void            tick(const shared_ptr<MyGL>& gl);
protected:
    ResourceManager();
    template <class _Map> 
    requires std::_Is_specialization_v<_Map, unordered_map> &&  std::same_as<typename _Map::key_type, string>
    void            DeregisteredSpecificMapElement(_Map& map, const string& name);

signals:
    void            ShaderRegistered  (const std::string& name);
    void            ShaderDeregistered(const std::string& name);

/// Fields 数据域
private:
    shared_ptr<MyGL>                                        gl_                  = nullptr;

    unordered_map <string, unique_ptr<QTimer>>              ui_update_timer_map_ = {};
    unordered_map <string, QOpenGLWidget*>                  draw_ui_map_         = {};
    unordered_map <string, unique_ptr<RenderMesh>>          mesh_map_            = {};
    unordered_map <string, unique_ptr<MyShader>>            shader_map_          = {};
    unordered_map <string, Texture>                         texture_map_         = {};

    list<TextureMsg>                                        texture_require_gl_  = {};
    list<CubemapMsg>                                        cubemap_require_gl_  = {};
    list<string>                                            mesh_require_gl_     = {};
    list<string>                                            shader_require_gl_   = {};
};

template <class _Map> 
requires std::_Is_specialization_v<_Map, unordered_map> && std::same_as<typename _Map::key_type, string>
void ResourceManager::DeregisteredSpecificMapElement(_Map& map, const string& name)
{
    auto iter = map.find(name);
    if (iter != map.end()) {
        map.erase(name);
    }
}

}
#endif // SCENEMANAGER_H
