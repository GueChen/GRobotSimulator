#ifndef CONVERSION_H
#define CONVERSION_H

#include <eigen3/Eigen/Dense>

#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>

#include <vector>

#ifdef _DEBUG
#include <iostream>
#endif // !

class Conversion
{
public:
    Conversion()  = delete;
    ~Conversion() = delete;
    
    static Eigen::Matrix4d toMat4d(const Eigen::Matrix4f& matrix) 
    {
        Eigen::Matrix4d ret_mat;        
        for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
        {
            ret_mat(i, j) = matrix(i, j);
        }
        return ret_mat;
    }
    
    static Eigen::Matrix3d toMat3d(const Eigen::Matrix3f& matrix) 
    {
        Eigen::Matrix3d ret_mat;        
        for(int i = 0; i < 3; ++i)
        for(int j = 0; j < 3; ++j)
        {
            ret_mat(i, j) = matrix(i, j);
        }
        return ret_mat;
    }

    static inline glm::vec3 fromVec3d(const Eigen::Vector3d & vec)
    {      
        return glm::vec3(vec.x(), vec.y(), vec.z());
    }

    static inline Eigen::Vector3d toVec3d(const glm::vec3& vec) 
    {
        return Eigen::Vector3d(vec.x, vec.y, vec.z);
    }
    
    static inline Eigen::Vector3d toVec3d(const Eigen::Vector3f& vec) 
    {
        return Eigen::Vector3d(vec.x(), vec.y(), vec.z());
    }

    static inline Eigen::Vector3f toVec3f(const glm::vec3& vec) 
    {
        return Eigen::Vector3f(vec.x, vec.y, vec.z);
    }

    static inline Eigen::Vector3f toVec3f(const Eigen::Vector3d& vec)
    {
        return Eigen::Vector3f(vec.x(), vec.y(), vec.z());
    }

    static inline glm::vec3 fromVec3f(const Eigen::Vector3f& vec) 
    {
        return glm::vec3(vec.x(), vec.y(), vec.z());
    }

    static inline Eigen::Matrix3f toMat3f(const glm::mat3& matrix)
    {
        return Eigen::Matrix3f(glm::value_ptr(matrix));
    }

    static inline glm::mat3 fromMat3f(const Eigen::Matrix3f& matrix)
    {
        return glm::mat3(matrix(0, 0), matrix(1, 0), matrix(2, 0), 
                         matrix(0, 1), matrix(1, 1), matrix(2, 1),
                         matrix(0, 2), matrix(1, 2), matrix(2, 2));
    }

    static Eigen::Matrix4d toMat4d(const glm::mat4 & matrix){
        Eigen::Matrix4f float_temp(glm::value_ptr(matrix));
        Eigen::Matrix4d temp = Eigen::Matrix4d::Identity();
        for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
        {
            temp(i, j) = float_temp(i, j);
        }                    
        return temp;
    }

    static glm::mat4 fromMat4d(const Eigen::Matrix4d & matrix){
        glm::mat4 mat(1.0f);   
        for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
        {
            mat[j][i] = matrix(i, j);
        }     
        return mat;
    }

    static inline Eigen::Matrix4f toMat4f(const glm::mat4& matrix) 
    {
        return Eigen::Matrix4f(glm::value_ptr(matrix));
    }

    static inline Eigen::Matrix4f toMat4f(const Eigen::Matrix4d& matrix) 
    {
        Eigen::Matrix4f ret_mat = Eigen::Matrix4f::Identity();
        for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
        {
            ret_mat(i, j) = matrix(i, j);
        }                    
        return ret_mat;
    }

    static inline glm::mat4 fromMat4f(const Eigen::Matrix4f& matrix)
    {
        return glm::mat4(matrix(0, 0), matrix(1, 0), matrix(2, 0), matrix(3, 0),
					     matrix(0, 1), matrix(1, 1), matrix(2, 1), matrix(3, 1),
					     matrix(0, 2), matrix(1, 2), matrix(2, 2), matrix(3, 2),
					     matrix(0, 3), matrix(1, 3), matrix(2, 3), matrix(3, 3));
    }

   
};

class STLUtils {
public:
    static Eigen::Matrix4f toMat4f(const std::vector<float>& t3_so3) {
        assert(t3_so3.size() == 6 && "STLUtils::toMat4f Input Size Not Match\n");
        Eigen::Affine3f mat = Eigen::Affine3f::Identity();
        Eigen::Vector3f t3(t3_so3[0], t3_so3[1], t3_so3[2]), so3(t3_so3[3], t3_so3[4], t3_so3[5]);
        mat.translate(t3).rotate(Eigen::AngleAxisf(so3.norm(), so3.normalized()));
        return mat.matrix();
    }

    static inline Eigen::Vector3f toVec3f(const std::vector<float>& v3) {
        assert(v3.size() == 3 && "STLUtils::toVec3 Input Size Not Match\n");
        return Eigen::Vector3f(v3[0], v3[1], v3[2]);
    }

    static inline glm::vec3 toGlmVec3(const std::vector<float>& v3) {
        assert(v3.size() == 3 && "STLUtils::toGlmVec3 Input Size Not Match\n");
        return glm::vec3(v3[0], v3[1], v3[2]);
    }

    static inline std::vector<float> fromDynVecf(const Eigen::Vector<float, Eigen::Dynamic>& vec) {
        return std::vector<float>{vec.begin(), vec.end()};
    }
private:
    STLUtils()  = delete;
    ~STLUtils() = delete;
};


#endif // CONVERSION_H
