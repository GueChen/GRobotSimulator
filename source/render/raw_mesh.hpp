/**
 *  @file  	raw_mesh.hpp
 *  @brief 	original mesh structure for rendring.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Apri 26, 2022
 **/
#ifndef _RAW_MESH_HPP
#define _RAW_MESH_HPP

#include "render/vertex.hpp"
#include "render/rendering_datastructure.hpp"
#include "render/texture.hpp"

#include <vector>

namespace GComponent {

	struct RawMesh {
		std::vector<Vertex>         Vertices;
		std::vector<Triangle>       Indices;
		std::vector<Texture>        Textures;
	};
}

#endif
