#ifndef __QCONVERSION_HPP
#define __QCONVERSION_HPP

#include <Eigen/Dense>
#include <QtGui/QVector3D>

class QConversion {
public:
	QConversion()  = delete;
	~QConversion() = delete;

	static inline Eigen::Vector3f toVec3f(const QVector3D& vec) {		
		return Eigen::Vector3f(vec[0], vec[1], vec[2]);
	}

	static inline QVector3D fromVec3f(const Eigen::Vector3f& vec) {
		return QVector3D(vec(0), vec(1), vec(2));
	}
};

#endif // !QCONVERSION_HPP

