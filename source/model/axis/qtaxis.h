/**
 *  @file	qtaxis.h
 *  @brief	Axis Resource File used for Graphic Program implementations etc.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date	Apri 26, 2022
 **/
#ifndef _QTAXIS_H
#define _QTAXIS_H

#include  "render/rendermesh.h"
#include  "render/myshader.h"
#include  "model/model.h"

#include <GComponent/GNumerical.hpp>

#include <memory>
#include <vector>

enum class AxisMode {
	Translation = 0,
	Rotation    = 1,
	Scale		= 2
};

enum class AxisSelected {
	xAxis = 0,
	yAxis = 1,
	zAxis = 2,
	None  = 3
};

namespace GComponent {
using std::vector;

class QtGLAbstractAxis : public Model {
public:
	QtGLAbstractAxis() { shader_ = "axis"; };
	virtual ~QtGLAbstractAxis() {}
	unsigned		    GetStridedSize();

	inline void			SetAxisSelected(AxisSelected which) { selected_which_ = which; }
	inline AxisSelected GetAxisSelected() const				{ return selected_which_; }

	void Init(int segments, float radius);
	void tickImpl(float delta_time) override;
	[[deprecated]]
	void Draw();
protected:
	void setShaderProperty(MyShader& shader) override;

	void SetupXaxisCircle(int segments, float radius, float fixed_x, vector<Vertex>&);
	void SetupYaxisVertex(const int count, vector<Vertex>&);
	void SetupZaxisVertex(const int count, vector<Vertex>&);

	void LinkTriangle(int strided, int base, int i1, int i2, int i3, vector<Triangle>&);
	void LinkSquare(int strided, int base, int i1, int i2, int i3, int i4, vector<Triangle>&);

	virtual vector<Vertex>   SetupVertexData(int, float) = 0 {}
	virtual vector<Triangle> SetupIndexData(int)	     = 0 {}

protected:
	AxisSelected  selected_which_ = AxisSelected::None;
};

class QtGLTranslationAxis : public QtGLAbstractAxis {
public:
	QtGLTranslationAxis();
	~QtGLTranslationAxis() = default;
protected:
	vector<Vertex>   SetupVertexData(int, float)  override;
	vector<Triangle> SetupIndexData(int)		  override;
private:
	static size_t count;
};


class QtGLRotationAxis : public QtGLAbstractAxis {
public:
	QtGLRotationAxis();
	~QtGLRotationAxis() = default;
protected:
	vector<Vertex>   SetupVertexData(int, float)  override;
	vector<Triangle> SetupIndexData(int)		  override;
private:
	static size_t count;
};

class QtGLScaleAxis : public QtGLAbstractAxis {
public:
	QtGLScaleAxis();
	~QtGLScaleAxis() = default;
protected:
	vector<Vertex>   SetupVertexData(int, float)  override;
	vector<Triangle> SetupIndexData(int)		  override;
private:
	static size_t count;
};


}

#endif // !_QTAXIS_H

