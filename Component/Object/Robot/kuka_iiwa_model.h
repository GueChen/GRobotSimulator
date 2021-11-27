#ifndef KUKA_IIWA_MODEL_H
#define KUKA_IIWA_MODEL_H
#include <memory>
#include "Component/Object/model.h"

namespace GComponent{
    class MyGL;
    class Mesh;
    class Revolute;
    class MyShader;

    using std::unique_ptr;
    using std::shared_ptr;
    using std::vector;
    using std::array;

    class KUKA_IIWA_MODEL:public Model
    {
    /// 成员函数 Member Functions
    public:
        KUKA_IIWA_MODEL(mat4 transform = mat4(1.0f));
        ~KUKA_IIWA_MODEL() = default;
        void Draw(MyShader * shader) override;
        void setColor(const vec3 & color);

        static void setGL(const shared_ptr<MyGL> & other);
    private:
        void Draw(MyShader* shader, Model * next);
        void InitializeResource();
        static void InsertMeshResource(const string & key, const string & source);

    /// 数据域 Fields
    public:
        vector<shared_ptr<Revolute>> Joints;
    private:
        vec3 _color = vec3(1.0f);
        // FIXME：单一窗口下资源优化的最佳方式，但多窗口下可能会存在隐患
        static bool hasInit;
        static unordered_map<string, unique_ptr<Mesh>> meshResource;
    };


}

#endif // KUKA_IIWA_MODEL_H
