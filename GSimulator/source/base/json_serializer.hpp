#ifndef __JSON_SERIALIZER_HPP
#define __JSON_SERIALIZER_HPP

#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>

#include <Eigen/Dense>

namespace GComponent {

class JSonSerializer {
public:
	JSonSerializer()  = delete;
	~JSonSerializer() = delete;

	static QJsonArray Serialize(const Eigen::Vector3f& vec) {
		QJsonArray obj;
		obj.append(vec.x());
		obj.append(vec.y());
		obj.append(vec.z());
		return obj;
	}

	static QJsonArray Serialize(const Eigen::Vector<float, 6>& vec) {
		QJsonArray obj;
		for (auto& val : vec) obj.append(val);
		return obj;
	}

	static QJsonArray Serialize(const Eigen::Matrix4<float>& mat) {
		QJsonArray obj;
		for (auto& row : mat.rowwise()) {
			QJsonArray row_obj;
			for (auto& val : row) {
				row_obj.append(val);
			}
			obj.append(row_obj);
		}
		return obj;
	}

	static QJsonArray Serialize(const Eigen::Matrix3<float>& mat) {
		QJsonArray obj;
		for (auto& row : mat.rowwise()) {
			QJsonArray row_obj;
			for (auto& val : row) {
				row_obj.append(val);
			}
			obj.append(row_obj);
		}
		return obj;
	}
};
}

#endif // !__GSERIALIZE_HPP

