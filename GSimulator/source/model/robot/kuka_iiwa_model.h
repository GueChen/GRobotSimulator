#ifndef KUKA_IIWA_MODEL_H
#define KUKA_IIWA_MODEL_H

#include "GComponent/gtransform.hpp"

#include "model/model.h"

#include <eigen3/Eigen/Dense>

namespace GComponent{

/// Forward Declaration
class RenderMesh;
class Revolute;
class MyShader;

/// STL Declaration
using std::map;
using std::array;
using std::vector;
using std::unique_ptr;
using std::shared_ptr;

class KUKA_IIWA_MODEL:public Model
{
/// ��Ա���� Member Functions
public:
/// ���캯�� Constructors
    explicit KUKA_IIWA_MODEL(Mat4 transform = Mat4::Identity());
    ~KUKA_IIWA_MODEL() = default;

/// Tick Functions
    void tickImpl(float delta_time) override;

/// ��ͼ���� Drawing Functions
    void setColor(const Vec3 & color);

   
private:
    void InitializeModelResource();
    void InitializeMeshResource();

/// ������ Fields
private:
// ��ǰ��һ��ɫ��ɫ���µ���ʾ��ɫ
    Vec3            _color     = Vec3::Ones();
    static bool is_init_;
    
    static int  count;

};


}

#endif // KUKA_IIWA_MODEL_H
