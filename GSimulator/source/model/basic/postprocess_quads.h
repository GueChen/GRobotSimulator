/**
 *  @file  	quads.h
 *  @brief 	a basic quads shape for post process.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	July 6th, 2022
 **/
#ifndef __POSTPROCESS_QUADS_H
#define __POSTPROCESS_QUADS_H

#include "model/model.h"

#include "render/rendering_datastructure.hpp"

#include <vector>

namespace GComponent {
class PostprocessQuads: public Model
{
public:
	PostprocessQuads();
	~PostprocessQuads() = default;

	void			Draw();
	void            tickImpl(float delta_time) override;

private:
	static const std::vector<Vertex>	vertices;
	static const std::vector<Triangle>	indeces;
};
}

#endif // !__POSTPROCESS_QUADS_H
