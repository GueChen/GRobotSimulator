/**
 *  @file  	json_serializer.hpp
 *  @brief 	This class used for some common datastructure serializer and deserializer
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	May 13th, 2023
 **/
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

class JsonDeserializer {
public:
	JsonDeserializer()  = delete;
	~JsonDeserializer() = delete;

	static Eigen::Vector3f ToVec3f(const QJsonArray& obj) {
		return Eigen::Vector3f(obj[0].toDouble(), 
							   obj[1].toDouble(),
							   obj[2].toDouble());
	}

	static Eigen::Vector<float, 6> ToVec6f(const QJsonArray& obj) {
		Eigen::Vector<float, 6> vec;
		for (int i = 0; i < 6; ++i) {
			vec(i) = obj[i].toDouble();
		}
		return vec;
	}

	static Eigen::Matrix4f ToMat4f(const QJsonArray& obj) {
		Eigen::Matrix4f mat;
		for (int i = 0; i < 4; ++i) {
			QJsonArray row_obj = obj[i].toArray();
			for (int j = 0; j < 4; ++j) {
				mat(i, j) = row_obj[j].toDouble();
			}
		}
		return mat;
	}

	static Eigen::Matrix3f ToMat3f(const QJsonArray& obj) {
		Eigen::Matrix3f mat;
		for (int i = 0; i < 3; ++i) {
			QJsonArray row_obj = obj[i].toArray();
			for (int j = 0; j < 3; ++j) {
				mat(i, j) = row_obj[j].toDouble();
			}
		}
		return mat;
	}
};

}

#endif // !__GSERIALIZE_HPP

