/**
 *  @file  	scenemanager.h
 *  @brief 	The Resource Manager by this Class include shader¡¢mesh.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Apri 25, 2022
 **/

#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include "base/singleton.h"

#include "component/mesh_component.h"
#include "render/myshader.h"
#include "render/camera.hpp"

#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QtCore/QObject>
#include <QtCore/QTimer>

#include <unordered_map>
#include <vector>
#include <memory>
#include <string>

namespace GComponent {

/// Ç°ÖÃÉùÃ÷ Forward Declaration
class MyGL;

using std::string;
using std::vector;
using std::unique_ptr;
using std::shared_ptr;
using std::unordered_map;

class SceneManager : public SingletonBase<SceneManager>, public QObject
{
    friend class SingletonBase<SceneManager>;
    NonCoyable(SceneManager)

public:
    ~SceneManager();

    void RegisteredMesh(const string& name, MeshComponent* raw_ptr_mesh);
    void DeregisteredMesh(const string& name);
    MeshComponent* GetMeshByName(const string& name);

    void RegisteredShader(const string& name, MyShader* raw_ptr_shader);
    void DeregisteredShader(const string& name);
    MyShader* GetShaderByName(const string& name);
    
    void RegisteredUIHandle(const string& name, QOpenGLWidget* ui_handle);
    void DeregisteredUIHandle(const string& name);
    QOpenGLWidget* GetUISurfaceByName(const string& name);

    void SetGL(const shared_ptr<MyGL>& gl);

    void tick(float delta_ms);
protected:
    SceneManager();

private:
    

    unordered_map <string, unique_ptr<QTimer>>              ui_update_timer_map_ = {};
    unordered_map <string, QOpenGLWidget*>                  draw_ui_map_         = {};
    unordered_map <string, unique_ptr<MeshComponent>>       mesh_map_            = {};
    unordered_map <string, unique_ptr<MyShader>>            shader_map_          = {};


};

}
#endif // SCENEMANAGER_H
