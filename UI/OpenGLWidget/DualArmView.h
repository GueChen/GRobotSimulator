#ifndef DUALARMVIEW_H
#define DUALARMVIEW_H

#include <QOpenGLWidget>
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
    class Camera;
    class MyShader;
    class BaseGrid;
    class Mesh;
    class Model;
    class Joint;
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
        void deleteMesh(const string & resource);
        void addMesh(const string & resource);
        void initModel();

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
        /* 计时器 */
        unique_ptr<QTimer> timer;
        /* 网格组件 */
        unique_ptr<BaseGrid> grid;
        /* Mesh 哈希表 */
        unordered_map<string, unique_ptr<Mesh>> mTable;
        /* 机器人模型 */
        unique_ptr<DUAL_ARM_PLATFORM> robot;
    };
}

#endif // DUALARMVIEW_H
