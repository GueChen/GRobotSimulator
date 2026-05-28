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
// TODO: ʹ�� ����ģʽ �Ըö�����иĽ�
// TODO: �����һ�����෽���������

class BaseGrid
{
private:
    /* ��ʽ�ߴ��� */
    int num;
    float gridSize;

    /* ��Դ������ */
    unsigned VAO, VBO, EBO;
    shared_ptr<MyGL> gl;

    /* ��ʼ����־ */
    bool isInit = false;

public:
    /* ���캯������������ */
    BaseGrid(int n, float size = 0.05f);
    ~BaseGrid() = default;

    /* GL ���ú��� */
    void SetGL(shared_ptr<MyGL> other);
    void SetRhiDevice(shared_ptr<IRhiDevice> rhi_device);

    /* ��ͼ�ӿ� */
    void Draw();

private:
    /* ��ʼ������ */
    void GLBufferInitialize();

    /* ���������㺯�� */
    static vector<vec3> GetGridVertexLocation(int num , float size);
    static vector<Line> GetGridEdge(int num);
};


}


#endif // BASEGRID_H
