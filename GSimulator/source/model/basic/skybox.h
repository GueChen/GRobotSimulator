#ifndef _SKYBOX_H
#define _SKYBOX_H

#include  "render/rendermesh.h"
#include  "model/model.h"

#include <vector>
#include <string>

namespace GComponent {
using std::vector;
using std::string_view;

class SkyBox :public Model {
public:
	SkyBox();	
	~SkyBox();

	void Draw();
	void tickImpl(float delta_time) override;

protected:
	void CheckAndRegisteredResource();

	vector<Vertex>		SetupVertexData();
	vector<Triangle>	SetupTrangle();

/// fields
private:
    unsigned cube_texture_id_	=	0;

/// static fields
private:
	static unsigned						count;
    static const vector<float>			Vertices;
	static const vector<string_view>	Textures;
};

}

#endif // !_SKYBOX_H
