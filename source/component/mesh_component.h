#ifndef MESH_H
#define MESH_H

#include "render/raw_mesh.hpp"

#include <vector>
#include <memory>

namespace GComponent {

class MyGL;

class MeshComponent
{
    /// 数据域 Fields
    protected:
        RawMesh mesh_datas_;

        unsigned VAO, VBO, EBO;
        bool HaveSetup = false;

    public:
        MeshComponent() = default;
        MeshComponent(std::vector<Vertex>   vertices,
                      std::vector<Triangle> indices,
                      std::vector<Texture>  textures);
        virtual ~MeshComponent();

        virtual void Draw();
        
        void setGL(const std::shared_ptr<MyGL> & other);

        inline unsigned getElementSize() const { return mesh_datas_.Indices.size(); }
        inline unsigned getVertexSize()  const { return mesh_datas_.Vertices.size(); }
        inline unsigned getTextureSize() const { return mesh_datas_.Textures.size(); }

    private:
        void CheckClearGL();
        void setupMesh();
        std::shared_ptr<MyGL> gl;
};

}


#endif // MESH_H
