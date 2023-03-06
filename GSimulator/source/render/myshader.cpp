#include "render/myshader.h"

#include "render/mygl.hpp"

#ifdef _DEBUG
#include <iostream>
#include <format>
#endif 

using namespace GComponent;

MyShader::MyShader(QObject * parent,
                   const std::string & vertexPath,
                   const std::string & fragmentPath,
                   const std::string & geometryPath):
    QOpenGLShaderProgram(parent)
{
    addShaderFromSourceFile(QOpenGLShader::Vertex,   vertexPath.c_str());
    addShaderFromSourceFile(QOpenGLShader::Fragment, fragmentPath.c_str());
    if(geometryPath != "")
    {
        addShaderFromSourceFile(QOpenGLShader::Geometry, geometryPath.c_str());
    }  
}

MyShader::~MyShader() = default;

void MyShader::use() noexcept
{
    bind();
}

void MyShader::SetGL(std::shared_ptr<MyGL> other)
{
    gl = other;
    link();
    
    GLint num_uniforms = 0;
    gl->glGetProgramInterfaceiv(programId(), GL_UNIFORM, GL_ACTIVE_RESOURCES, &num_uniforms);
    std::cout << std::format("uniform num: {:}", num_uniforms);
    for (int i = 0; i < num_uniforms; ++i) {
        GLenum properties[] = { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION };
        GLint  values[3];
        gl->glGetProgramResourceiv(programId(), GL_UNIFORM, i, 3, properties, 3, NULL, values);
        
        GLint name_length = values[0];
        char* name_buffer = new char[name_length];
        gl->glGetProgramResourceName(programId(), GL_UNIFORM, i, name_length, NULL, name_buffer);

        std::cout << std::format("Uniform {:}: {:}\n", i, name_buffer);
        std::cout << std::format("Type: {:}\n", values[1]);
        std::cout << std::format("Location: {:}\n", values[2]);
        delete[] name_buffer;

    }
}

void MyShader::setBool(const std::string & name, bool value) noexcept
{
    gl->glUniform1i(gl->glGetUniformLocation(programId(), name.c_str()), (int)value);
}

void MyShader::setInt(const std::string& name, int value) noexcept
{    
    gl->glUniform1i(gl->glGetUniformLocation(programId(), name.c_str()), value);
}

void GComponent::MyShader::setUint(const std::string& name, unsigned value) noexcept
{
    gl->glUniform1ui(gl->glGetUniformLocation(programId(), name.c_str()), value);
}

void MyShader::setFloat(const std::string& name, float value) noexcept
{
    gl->glUniform1f(gl->glGetUniformLocation(programId(), name.c_str()), value);
}

void MyShader::setVec2(const std::string& name, float value[2]) noexcept
{
    gl->glUniform2fv(gl->glGetUniformLocation(programId(), name.c_str()), 1, value);
}
void MyShader::setVec2(const std::string& name, glm::vec2 value) noexcept
{
    gl->glUniform2fv(gl->glGetUniformLocation(programId(), name.c_str()), 1, value_ptr(value));
}

void MyShader::setVec4(const std::string& name, float value[4]) noexcept
{
    gl->glUniform4f(gl->glGetUniformLocation(programId(), name.c_str()), value[0], value[1], value[2], value[3]);
}

void MyShader::setVec3(const std::string& name, float value[3]) noexcept
{
    gl->glUniform3fv(gl->glGetUniformLocation(programId(), name.c_str()), 1, value);
}

void MyShader::setVec3(const std::string& name, glm::vec3 value) noexcept
{
    gl->glUniform3fv(gl->glGetUniformLocation(programId(), name.c_str()), 1, value_ptr(value));
}

void MyShader::setMat4(const std::string& name, glm::mat4 mat) noexcept
{
    gl->glUniformMatrix4fv(gl->glGetUniformLocation(programId(), name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}
