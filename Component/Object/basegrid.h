#ifndef BASEGRID_H
#define BASEGRID_H
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "Component/GStruct.hpp"

namespace GComponent {
    using std::shared_ptr;
    using std::vector;
    using vec3 = glm::vec3;

    class MyGL;
    // TODO: 使用 单例模式 对该对象进行改进
    class BaseGrid
    {
    public:
        BaseGrid(int n, float size = 0.05f);
        ~BaseGrid() = default;
        void setGL(shared_ptr<MyGL> other);
        void Draw();
    private:
        int num;
        float gridSize;

        unsigned VAO, VBO, EBO;
        shared_ptr<MyGL> gl;
        bool isInit = false;
        void GLBufferInitialize();

        static vector<vec3> GetGridVertexLocation(int num , float size);
        static vector<Line> GetGridEdge(int num);
    };
}


#endif // BASEGRID_H
