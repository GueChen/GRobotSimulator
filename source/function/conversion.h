#ifndef CONVERSION_H
#define CONVERSION_H

#include<glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include<eigen3/Eigen/Dense>
#include <iostream>
class Conversion
{

public:
    Conversion()  = delete;
    ~Conversion() = delete;
    static glm::vec3 Vec3Eigen2Glm(const Eigen::Vector3d & vec){
        Eigen::Vector3d temp = ConvertMat3* vec;
        return glm::vec3(temp.x(), temp.y(), temp.z());
    }

    static Eigen::Matrix4d Mat4Glm2Eigen(const glm::mat4 & matrix){
        Eigen::Matrix4f float_temp(glm::value_ptr(matrix));
        Eigen::Matrix4d temp = Eigen::Matrix4d::Identity();

        for(int i = 0; i < 4; ++i){
            for(int j = 0; j < 4; ++j){
                temp(i, j) = float_temp(i, j);
            }            
        }
        return ConvertMat4.inverse() * temp *ConvertMat4;
    }

    static glm::mat4 Mat4Glm2Eigen(const Eigen::Matrix4d & matrix){
        glm::mat4 mat(1.0f);
        Eigen::Matrix4d temp = ConvertMat4 * matrix;
        for(int i = 0; i < 4; ++i){
            for(int j = 0; j <4; ++j){
                mat[j][i] = temp(i, j);
            }
        }
        return mat;
    }
//    static glm::mat4 Mat4Eigen2Glm(const Eigen::Matrix4d & matrix){
//        glm::mat4 ret_val;
//    }
    static Eigen::Matrix<double, 3, 3> ConvertMat3;
    static Eigen::Matrix<double, 4, 4> ConvertMat4;
};


#endif // CONVERSION_H
