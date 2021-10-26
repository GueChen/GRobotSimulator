#ifndef MODELVIEW_H
#define MODELVIEW_H

#include <QOpenGLWidget>
#include <memory>

class QTimer;

namespace GComponent {
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

    class ModelView : public QOpenGLWidget
    {
        Q_OBJECT
    /// 成员函数 Member Functions

    public:
        /// 构造函数 Constructors
        explicit ModelView(QWidget *parent = nullptr);
        // FIXME:析构函数未设计, 赋值构造函数等未删除
        ~ModelView() = default;

    protected:
        /// GL 相关初始化函数重写
        void initializeGL() override;
        void resizeGL(int w, int h) override;
        void paintGL() override;
        void cleanup();

        /// 窗口控件相关重写
        void wheelEvent(QWheelEvent * event) override;
        void mouseMoveEvent(QMouseEvent * evnet) override;
        void mousePressEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        void dragEnterEvent(QDragEnterEvent * event) override;
        void dropEvent(QDropEvent * evnt) override;
    private:
        /// 管理素材相关函数
        void deleteMesh(const string & resource);
        void addMesh(const string & resource);
        void initModel();

    /// 数据域 Fields
    private:
        /// openGL 相关类 OpenGL Atributes Class
        /* gl 函数库 */
        shared_ptr<MyGL> gl;
        unsigned int uMBO;
        bool GLHasInit      = false;
        /* 相机参数类 */
        unique_ptr<Camera> camera;
        bool LeftClick      = false,
             RightClick     = false,
             MiddleClick    = false;
        QPointF lastMousePos;
        /* 着色器类 */
        MyShader * curShader;
        unordered_map<
            std::string,
            unique_ptr<MyShader>> shaderMap;
        /* 定时器 */
        unique_ptr<QTimer> timer;
        /* 地面网格 */
        unique_ptr<BaseGrid> grid;
        /* 网格管理哈希表 */
        unordered_map<string, unique_ptr<Mesh>> mTable;

        vector<std::shared_ptr<Model>> models;
    };
}
#endif // MODELVIEW_H
