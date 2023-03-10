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
    if (!init_) {
        GLint num_uniforms = 0;
        gl->glGetProgramInterfaceiv(programId(), GL_UNIFORM, GL_ACTIVE_RESOURCES, &num_uniforms);        
        std::cout << std::format("{:<30} uniform num: {:<2}\n", name_, num_uniforms);
        for (int i = 0; i < num_uniforms; ++i) {
            
            GLenum properties[] = { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION };
            GLint  values[3];
            gl->glGetProgramResourceiv(programId(), GL_UNIFORM, i, 3, properties, 3, NULL, values);

            GLint name_length = values[0];
            std::unique_ptr<char[]> name_buffer(new char[name_length]);
            
            gl->glGetProgramResourceName(programId(), GL_UNIFORM, i, name_length, NULL, name_buffer.get());

            std::cout << std::format("variables {:<2}: name -- {:<30} | type -- {:<20} | location -- {:} \n", 
                i, name_buffer.get(), GetTypeName(values[1]), values[2]);
            if (values[2] >= 0) {
                Property variable;
                variable.name     = name_buffer.get();  
                variable.type     = GetTypeName(values[1]);
                variable.location = values[2];

            }
        }
        std::cout << "______________________________________________\n";
        std::cout << "______________________________________________\n";
        init_ = true;
    }
}

void GComponent::MyShader::setUniformValue(int location, int value)
{
    QOpenGLShaderProgram::setUniformValue(location, value);
}

void GComponent::MyShader::setUniformValue(int location, unsigned value)
{
    QOpenGLShaderProgram::setUniformValue(location, value);
}

void GComponent::MyShader::setUniformValue(int location, float value)
{
    QOpenGLShaderProgram::setUniformValue(location, value);
}

void GComponent::MyShader::setUniformValue(int location, bool value)
{
    QOpenGLShaderProgram::setUniformValue(location, static_cast<int>(value));
}

void GComponent::MyShader::setUniformValue(int location, glm::vec2 value)
{
    QOpenGLShaderProgram::setUniformValue(location, value[0], value[1]);
}

void GComponent::MyShader::setUniformValue(int location, glm::vec3 value)
{
    QOpenGLShaderProgram::setUniformValue(location, value[0], value[1], value[2]);
}

void GComponent::MyShader::setUniformValue(int location, glm::vec4 value)
{
    QOpenGLShaderProgram::setUniformValue(location, value[0], value[1], value[2], value[3]);
}

void GComponent::MyShader::setUniformValue(int location, glm::mat4 mat)
{
    gl->glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
}

void MyShader::setBool(const std::string & name, bool value) noexcept
{
    gl->glUniform1i(gl->glGetUniformLocation(programId(), name.c_str()), (int)value);
}

void MyShader::setInt(const std::string& name, int value) noexcept
{    
    gl->glUniform1i(gl->glGetUniformLocation(programId(), name.c_str()), value);
}

void GComponent::MyShader::setUint(int location, unsigned int value) noexcept
{
    gl->glUniform1ui(location, value);
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

std::string GComponent::MyShader::GetTypeName(GLenum type)
{
    switch (type) {
    case GL_BOOL:               return "bool";
    case GL_INT:                return "int";
    case GL_UNSIGNED_INT:       return "unsigned int";
    case GL_FLOAT:              return "float";
    case GL_DOUBLE:             return "double";
    case GL_FLOAT_VEC2:         return "vec2";
    case GL_FLOAT_VEC3:         return "vec3";
    case GL_FLOAT_VEC4:         return "vec4";
    case GL_FLOAT_MAT2:         return "mat2";
    case GL_FLOAT_MAT3:         return "mat3";
    case GL_FLOAT_MAT4:         return "mat4";
    case GL_SAMPLER_2D:         return "sampler2D";
    case GL_SAMPLER_CUBE:       return "samplerCUBE";
    case GL_SAMPLER_2D_ARRAY:   return "sampler2DArray";
    default:
        return "?";
    }
}
