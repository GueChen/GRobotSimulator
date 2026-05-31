#ifndef BASEGRID_H
#define BASEGRID_H

#include "render/rendering_datastructure.hpp"
#include "render/rhi/rhi_device.h"

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace GComponent {


using std::vector;
using std::shared_ptr;
using vec3 = glm::vec3;


class MyGL;
// TODO: Improve this object with a better construction pattern.
// TODO: Extract common behavior into a base type.

class BaseGrid
{
private:
    /* grid size */
    int num;
    float gridSize;

    /* resource handles */
    unsigned VAO, VBO, EBO;
    shared_ptr<MyGL> gl;

    /* initialization flag */
    bool isInit = false;

public:
    /* construction and destruction */
    BaseGrid(int n, float size = 0.05f);
    ~BaseGrid() = default;

    /* GL setup */
    void SetGL(shared_ptr<MyGL> other);
    void SetRhiDevice(shared_ptr<IRhiDevice> rhi_device);

    /* draw interface */
    void Draw();

private:
    /* initialization */
    void GLBufferInitialize();

    /* grid generation helpers */
    static vector<vec3> GetGridVertexLocation(int num , float size);
    static vector<Line> GetGridEdge(int num);
};


}


#endif // BASEGRID_H
