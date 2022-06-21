#ifndef _SKYBOX_H
#define _SKYBOX_H

#include  "render/rendermesh.h"
#include  "render/myshader.h"
#include  "model/model.h"

#include <vector>
#include <string>

namespace GComponent {
using std::vector;
using std::string_view;

class SkyBox :public Model {
public:
	SkyBox();
	~SkyBox() = default;

	void tickImpl(float delta_time) override;

protected:
	void setShaderProperty(MyShader& shader) override;

	vector<Vertex>		SetupVertexData();
	vector<Triangle>	SetupTrangle();

private:
    unsigned cube_texture_id_;
    static const vector<float>			VertexData;
	static const vector<string_view>	TexturePath;
};

}

#endif // !_SKYBOX_H
