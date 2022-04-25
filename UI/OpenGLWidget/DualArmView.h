#ifndef DUALARMVIEW_H
#define DUALARMVIEW_H

#include <QtOpenGLWidgets/QOpenGLWidget>

#include <memory>

class QTimer;

namespace GComponent {

using std::pair;
using std::array;
using std::vector;
using std::string;
using std::unique_ptr;
using std::shared_ptr;
using std::unordered_map;

class MyGL;
class Mesh;
class Model;
class SimplexModel;
class Joint;
class Camera;
class MyShader;
class BaseGrid;
class KUKA_IIWA_MODEL;
class DUAL_ARM_PLATFORM;

using JointsPair = pair<array<Joint*, 7>, array<Joint*, 7>>;

class DualArmView : public QOpenGLWidget
{
    Q_OBJECT
/// 成员函数 Member Functions
public:
    /// 构造函数 Constructors
    explicit DualArmView(QWidget *parent = nullptr);
    // FIXME: 默认构造函数可能需要修改
    ~DualArmView() = default;

    /// 对外接口 Interface
    template<class _Derived>
    void addSimplexModel(const string&name, unique_ptr<_Derived> &&);
    void deleteSimplexModel(const string& name);
    void clearSimplexModel();

    /// 设置获取函数 Getter and Setter
    // TODO: 后期考虑如何把接口修改的好一些
    JointsPair getJoints();
    KUKA_IIWA_MODEL* getLeftRobot() const;
    KUKA_IIWA_MODEL* getRightRobot() const;


protected:
    /// GL 重写函数
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void cleanup();

    /// 事件函数
    void wheelEvent(QWheelEvent * event) override;
    void mouseMoveEvent(QMouseEvent * evnet) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent * event) override;
    void dropEvent(QDropEvent * evnt) override;

private:
    /// 模型函数
    // TODO: 目前该部分没用应用，考虑修改
    void deleteMesh(const string & resource);
    void addMesh(const string & resource);

    /// 辅助函数 传递 GLContext
    void setGL();

/// 数据域 Fields
private:
    /// openGL 属性 OpenGL Atributes Class
    /* gl 函数指针*/
    shared_ptr<MyGL> gl;
    unsigned int uMBO;
    bool GLHasInit      = false;
    /* 相机模型 */
    unique_ptr<Camera> camera;
    bool LeftClick      = false,
         RightClick     = false,
         MiddleClick    = false;
    QPointF lastMousePos;

    /* Shader 着色器 */
    MyShader * curShader;
    /* 着色器哈希表 */
    unordered_map<
        std::string,
        unique_ptr<MyShader>> shaderMap;

    /* 网格组件 */
    unique_ptr<BaseGrid> grid;
    /* Mesh 哈希表 */
    unordered_map<string, unique_ptr<Mesh>> meshTable;
    /* 机器人模型 */
    unique_ptr<DUAL_ARM_PLATFORM> robot;
    /* 绘图模型 */
    unordered_map<string, unique_ptr<SimplexModel>> modelTable;
    /* 计时器 */
    unique_ptr<QTimer> timer;

};

}

#endif // DUALARMVIEW_H
