/**
 *  @file  Axis.h
 *  @brief Axis Resource File used for Graphic Program etc.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date Apri 16, 2022
 **/

#pragma once
#include "GRendering/GObject.hpp"
#include <vector>

class AbstractAxis : public GMeshObject {
protected:
	unsigned						  EBO		 = 0;
	bool							  have_init_ = false;
	std::vector<GComponent::Vertex>   vertices_;
	std::vector<GComponent::Triangle> indicies_;

public:
	AbstractAxis() = default;
	virtual ~AbstractAxis() override { glDeleteVertexArrays(1, &ID); glDeleteBuffers(1, &VBO); glDeleteBuffers(1, &EBO); };
	inline unsigned GetStridedSize() { return indicies_.size() / 3; }

	void Init(int segments, float radius);
	void Draw(MyShader& shader) const noexcept override;

protected:
	void SetupXaxisCircle(int segments, float radius, float fixed_x);
	void SetupYaxisVertex(const int count);
	void SetupZaxisVertex(const int count);

	void LinkTriangle(int strided, int base, int i1, int i2, int i3);
	void LinkSquare(int strided, int base, int i1, int i2, int i3, int i4);

	virtual void SetupVertexData(int, float) = 0 {}
	virtual void SetupIndexData(int)		 = 0 {}
};

class TranslationAxis : public AbstractAxis {
public:
	TranslationAxis();
	~TranslationAxis() = default;
protected:
	void SetupVertexData(int, float) override;
	void SetupIndexData(int)		 override;
};

class RotationAxis :public AbstractAxis {
public:
	RotationAxis();
	~RotationAxis() = default;
protected:
	void SetupVertexData(int, float) override;
	void SetupIndexData(int)		 override;
};

class ScaleAxis : public AbstractAxis {
public:
	ScaleAxis();
	~ScaleAxis() = default;
protected:
	void SetupVertexData(int, float) override;
	void SetupIndexData(int)		 override;
};

