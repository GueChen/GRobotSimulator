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

        unsigned VAO_               = 0;
        unsigned VBO_               = 0;
        unsigned EBO_               = 0;
        bool     is_setup_          = false;

        DrawMode draw_mode_         = DrawMode::Triangles;
        std::shared_ptr<MyGL> gl_   = nullptr;

    /// 成员函数 Methods
    public:
        RenderMesh() = default;
        RenderMesh(const std::vector<Vertex>&   vertices,
                   const std::vector<Triangle>& indices,
                   const std::vector<Texture>&  textures);
        virtual ~RenderMesh();

        virtual void Draw();
        
        void SetupRawMesh(const std::vector<Vertex>&   vertices,
                          const std::vector<Triangle>& indices,
                          const std::vector<Texture>&  textures);
        void SetupRawMesh(RawMesh&& raw_mesh_datas);

        void SetGL(const std::shared_ptr<MyGL> & other);
        
        /// <summary>
        /// 获取所持有的渲染单纯形的数目（通常为三角形）
        /// </summary>
        /// <returns></returns>
        inline size_t GetElementSize() const { return mesh_datas_.indices.size(); }

        /// <summary>
        /// 获取所持有的顶点数目
        /// </summary>
        /// <returns></returns>
        inline size_t GetVertexSize()  const { return mesh_datas_.vertices.size(); }

        /// <summary>
        /// 获取所持有的渲染纹理的数目
        /// </summary>
        /// <returns></returns>
        inline size_t GetTextureSize() const { return mesh_datas_.textures.size(); }
                    
        inline std::vector<Vertex>& GetVertexData() { return mesh_datas_.vertices; }
        
    private:
        void CheckClearGL();
        void SetupMesh();
       
};

}


#endif // MESH_H
