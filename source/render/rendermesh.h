#ifndef MESH_H
#define MESH_H

#include "render/raw_mesh.hpp"

#include <vector>
#include <memory>

namespace GComponent {

class MyGL;
enum DrawMode {
Points                          = 0x0000,
Lines                           = 0x0001,
LineLoop                        = 0x0002,
LineStrip                       = 0x0003,
Triangles                       = 0x0004,
TriangleStrip                   = 0x0005,
TriangleFan                     = 0x0006,
Quads                           = 0x0007,
QuadStrip                       = 0x0008,
Polygon                         = 0x0009
};

class RenderMesh
{
    /// 数据域 Fields
    protected:
        RawMesh mesh_datas_;

        unsigned VAO_, VBO_, EBO_;
        bool     is_setup_ = false;

        DrawMode draw_mode_ = DrawMode::Triangles;
    public:
        RenderMesh() = default;
        RenderMesh(std::vector<Vertex>   vertices,
                      std::vector<Triangle> indices,
                      std::vector<Texture>  textures);
        virtual ~RenderMesh();

        virtual void Draw();
        
        void setGL(const std::shared_ptr<MyGL> & other);

        inline size_t getElementSize() const { return mesh_datas_.Indices.size(); }
        inline size_t getVertexSize()  const { return mesh_datas_.Vertices.size(); }
        inline size_t getTextureSize() const { return mesh_datas_.Textures.size(); }

    private:
        void CheckClearGL();
        void setupMesh();
        std::shared_ptr<MyGL> gl;
};

}


#endif // MESH_H
