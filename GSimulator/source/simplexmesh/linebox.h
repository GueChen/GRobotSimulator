/**
 *  @file  	linebox.h
 *  @brief 	this class used for debugging display box model.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Mar 26th, 2022
 **/
#ifndef __GLINEBOX_H
#define __GLINEBOX_H

#include "simplexmesh/simplexmodel.hpp"

#include "render/rendering_datastructure.hpp"

#include <vector>

namespace GComponent {

class GLineBox : public SimplexModel {
public:
	GLineBox(const vec3& min_pos, const vec3& max_pos);
	~GLineBox() = default;

	void Draw(MyShader* shader = nullptr);
	void Update(const vec3& min_pos, const vec3& max_pos);

private: 
	void GLBufferInitialize() override;
private:
	Vertex vert_min;
	Vertex vert_max;

	std::vector<ColorVertex> verts;
};

}


#endif // !__GLINEBOX_H
