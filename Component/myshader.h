#ifndef MYSHADER_H
#define MYSHADER_H
#include <QOpenGLShaderProgram>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>

class QOpenGLFunctions_4_5_Core;
class MyShader:public QOpenGLShaderProgram
{
public:
    /**
     * Shader 构造函数，此处封装了 QOpenGLShaderProgram
     * @param vertexPath 顶点着色器路径
     * @param fragmentPath 片段着色器路径
     * @param geometryPath 几何着色器路径
     * */
    MyShader(QObject * parent,const std::string& vertexPath, const std::string& fragmentPath, const std::string & geometryPath = "");

    QOpenGLFunctions_4_5_Core * gl;

    virtual ~MyShader() = default;

    /// 调用 bind 直接使用 继承的 QOpenGLShaderProgram::bind() 效果相同
    void use() noexcept;
    void setGLFunctions(QOpenGLFunctions_4_5_Core * pGL);
    /// Set 系列函数，使用前请先使用 MyShader::use
    void setBool(const std::string & name, bool value) noexcept;
    void setInt(const std::string & name, int value) noexcept;
    void setFloat(const std::string& name, float value) noexcept;
    void setVec2(const std::string& name, float value[2]) noexcept;
    void setVec2(const std::string& name, glm::vec2 value) noexcept;
    void setVec4(const std::string& name, float value[4]) noexcept;
    void setVec3(const std::string& name, float value[3]) noexcept;
    void setVec3(const std::string& name, glm::vec3 value) noexcept;
    void setMat4(const std::string& name, glm::mat4 mat) noexcept;
};

#endif // MYSHADER_H
