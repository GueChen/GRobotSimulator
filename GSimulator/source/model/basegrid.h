#ifndef BASEGRID_H
#define BASEGRID_H

#include "render/rendering_datastructure.hpp"

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace GComponent {


using std::vector;
using std::shared_ptr;
using vec3 = glm::vec3;


class MyGL;
// TODO: 使用 单例模式 对该对象进行改进
// TODO: 抽象出一个大类方便后续管理

class BaseGrid
{
private:
    /* 格式尺寸项 */
    int num;
    float gridSize;

    /* 资源管理项 */
    unsigned VAO, VBO, EBO;
    shared_ptr<MyGL> gl;

    /* 初始化标志 */
    bool isInit = false;

public:
    /* 构造函数和析构函数 */
    BaseGrid(int n, float size = 0.05f);
    ~BaseGrid() = default;

    /* GL 设置函数 */
    void SetGL(shared_ptr<MyGL> other);

    /* 绘图接口 */
    void Draw();

private:
    /* 初始化函数 */
    void GLBufferInitialize();

    /* 网格辅助计算函数 */
    static vector<vec3> GetGridVertexLocation(int num , float size);
    static vector<Line> GetGridEdge(int num);
};


}


#endif // BASEGRID_H
