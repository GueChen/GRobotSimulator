#ifndef MESH_H
#define MESH_H
#include <vector>
#include <memory>
#include <Component/Geometry/Vertex.hpp>
#include <Component/Geometry/Texture.hpp>

namespace GComponent {

class MyGL;

class Mesh
{
    /// 数据域 Fields
    public:
        std::vector<Vertex>         Vertices;
        std::vector<unsigned>       Indices;
        std::vector<Texture>        Textures;

    private:
        unsigned VAO, VBO, EBO;
        bool HaveSetup = false;

    public:
        Mesh(std::vector<Vertex> vertices,
             std::vector<unsigned> indices,
             std::vector<Texture> textures);
        ~Mesh();

        void Draw();
        void setGL(const std::shared_ptr<MyGL> & other);

    private:
        unsigned NumElements;
        void CheckClearGL();
        void setupMesh();
        std::shared_ptr<MyGL> gl;
};

}


#endif // MESH_H
