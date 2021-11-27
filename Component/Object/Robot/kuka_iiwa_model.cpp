#include <glm/gtc/matrix_transform.hpp>
#include "Component/MyGL.hpp"
#include "kuka_iiwa_model.h"
#include "Component/Geometry/mesh.h"
#include "Component/Geometry/modelloader.h"
#include "Component/myshader.h"
#include "Component/Object/Robot/joint.h"

#define IIWASource(name) ("iiwa14_"#name".STL")

using namespace GComponent;
unordered_map<string, unique_ptr<Mesh>> KUKA_IIWA_MODEL::meshResource{};
bool KUKA_IIWA_MODEL::hasInit = false;

/**
 *
 *      Static Member Functions
 *
 ************************************/

/********************************************
 * <desc> KUKA_IIWA_MODEL::setGL                       </desc>
 * <param> other gl函数指针包含上下文信息                 </param>
 *
 * <attention> 调用该函数前务必调用 makeCurrent,
 *             传递的 gl 确保已经过 initialize           </attention>
 *
 ********************************************/
void KUKA_IIWA_MODEL::setGL(const shared_ptr<MyGL> &other)
{
    for(auto & [_, mesh]: meshResource)
    {
        mesh->setGL(other);
    }
}

void KUKA_IIWA_MODEL::InsertMeshResource(const string &key, const string &source)
{
    meshResource.insert(std::make_pair(
                            key,
                            std::make_unique<Mesh>(
                                ModelLoader::getMesh(sPathModel(source)))));
}

/**
 *
 *      Normal Member Functions
 *
 ************************************/
KUKA_IIWA_MODEL::KUKA_IIWA_MODEL(mat4 transform)
{
    _matrixModel = transform;
    InitializeResource();
}

void KUKA_IIWA_MODEL::InitializeResource()
{

    if(hasInit)
    {
        InsertMeshResource("Base", IIWASource(base));
        InsertMeshResource("Link1", IIWASource(link_1));
        InsertMeshResource("Link2", IIWASource(link_2));
        InsertMeshResource("Link3", IIWASource(link_3));
        InsertMeshResource("Link4", IIWASource(link_4));
        InsertMeshResource("Link5", IIWASource(link_5));
        InsertMeshResource("Link6", IIWASource(link_6));
        InsertMeshResource("Flansch", "flanschExten.STL");
    }

    vector<shared_ptr<Model>> models(7);
    for(int i = 0; i < 7; ++i)
    {
        models[i] = std::make_shared<Model>();
    }

    setMesh("Base");
    models[0]->setMesh("Link1");
    models[1]->setMesh("Link2");
    models[2]->setMesh("Link3");
    models[3]->setMesh("Link4");
    models[4]->setMesh("Link5");
    models[5]->setMesh("Link6");
    models[6]->setMesh("Flansch");

    models[0]->setAxis(vec3(0.0f, 1.0f, 0.0f));
    models[1]->setAxis(vec3(0.0f, 0.0f, 1.0f));
    models[2]->setAxis(vec3(0.0f, 1.0f, 0.0f));
    models[3]->setAxis(vec3(0.0f, 0.0f, -1.0f));
    models[4]->setAxis(vec3(0.0f, 1.0f, 0.0f));
    models[5]->setAxis(vec3(0.0f, 0.0f, 1.0f));
    models[6]->setAxis(vec3(0.0f, 1.0f, 0.0f));

    mat4 im = glm::identity<mat4>();

    models[5]->appendChild(models[6], glm::translate(im, vec3(0.0f, 0.0809f, 0.0607f)));
    models[4]->appendChild(models[5], glm::translate(im, vec3(0.0f, 0.2155f, -0.0607f)));
    models[3]->appendChild(models[4], glm::translate(im, vec3(0.0f, 0.1845f, 0.0f)));
    models[2]->appendChild(models[3], glm::translate(im, vec3(0.0f, 0.2155f, 0.0f)));
    models[1]->appendChild(models[2], glm::translate(im, vec3(0.0f, 0.2045f, 0.0f)));
    models[0]->appendChild(models[1], glm::translate(im, vec3(0.0f, 0.2025f, 0.0f)));
    appendChild(models[0], glm::translate(im, vec3(0.0f, 0.1575f, 0.0f)));


    Joints.resize(7);
    Joints[0] = std::make_shared<Revolute>(models[0]);
    Joints[1] = std::make_shared<Revolute>(models[1]);
    Joints[2] = std::make_shared<Revolute>(models[2]);
    Joints[3] = std::make_shared<Revolute>(models[3]);
    Joints[4] = std::make_shared<Revolute>(models[4]);
    Joints[5] = std::make_shared<Revolute>(models[5]);
    Joints[6] = std::make_shared<Revolute>(models[6]);

    hasInit = true;
}

void KUKA_IIWA_MODEL::Draw(MyShader * shader)
{
    shader->setVec3("color", _color);
    Draw(shader, this);
}

void KUKA_IIWA_MODEL::Draw(MyShader * shader, Model * next)
{
    shader->setMat4("model", next->getModelMatrix());
    meshResource[next->getMesh()]->Draw();
    for(auto & [_child, _]: next->getChildren())
    {
        Draw(shader, _child.get());
    }
}

void KUKA_IIWA_MODEL::setColor(const vec3 &color)
{
    _color = color;
}
