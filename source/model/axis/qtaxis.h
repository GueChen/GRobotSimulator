#ifndef _QTAXIS_H
#define _QTAXIS_H

#include  "model/model.h"
#include  "component/mesh_component.h"

enum class AxisSelected {
	xAxis = 0,
	yAxis = 1,
	zAxis = 2,
	None  = 3
};

namespace GComponent {
class QtGLAbstractAxis : public Model {
protected:
	MeshComponent mesh_component_;
public:
	QtGLAbstractAxis()	= default;
	virtual ~QtGLAbstractAxis() override {}
	inline unsigned GetStridedSize() { return mesh_component_.getElementSize() / 3; }

	void Init(int segments, float radius);
	void setGL(const shared_ptr<MyGL>& other);
	virtual void tick();

protected:
	void SetupXaxisCircle(int segments, float radius, float fixed_x, vector<Vertex>&);
	void SetupYaxisVertex(const int count, vector<Vertex>&);
	void SetupZaxisVertex(const int count, vector<Vertex>&);

	void LinkTriangle(int strided, int base, int i1, int i2, int i3, vector<Triangle>&);
	void LinkSquare(int strided, int base, int i1, int i2, int i3, int i4, vector<Triangle>&);

	virtual std::vector<Vertex> SetupVertexData(int, float) = 0 {}
	virtual std::vector<Triangle> SetupIndexData(int) = 0 {}
};

class QtGLTranslationAxis : public QtGLAbstractAxis {
public:
	QtGLTranslationAxis() = default;
	~QtGLTranslationAxis() = default;
protected:
	std::vector<Vertex>   SetupVertexData(int, float) override;
	std::vector<Triangle> SetupIndexData(int)		  override;
};

class QtGLRotationAxis : public QtGLAbstractAxis {
public:
	QtGLRotationAxis() = default;
	~QtGLRotationAxis() = default;
protected:
	std::vector<Vertex>   SetupVertexData(int, float) override;
	std::vector<Triangle> SetupIndexData(int)		  override;
};

class QtGLScaleAxis : public QtGLAbstractAxis {
public:
	QtGLScaleAxis() = default;
	~QtGLScaleAxis() = default;
protected:
	std::vector<Vertex>   SetupVertexData(int, float) override;
	std::vector<Triangle> SetupIndexData(int)		  override;
};

}

#endif // !_QTAXIS_H
