#ifndef MYSHADER_H
#define MYSHADER_H

#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>

#include <QtOpenGL/QOpenGLShaderProgram>

#include <memory>
#include <string>

#define PathVert(name) ("./Shader/Vertex/"#name"Vert.vert")
#define PathFrag(name) ("./Shader/Fragment/"#name"Frag.frag")
#define PathGeom(name) ("./Shader/Geom/"#name"Geom.geom")
#define PathVertString(name) ("./Shader/Vertex/"+ name +"Vert.vert")
#define PathFragString(name) ("./Shader/Fragment/" + name +"Frag.frag")

#define genShaderMap(name)                  \
std::make_pair((#name),                     \
std::make_unique<MyShader>(                 \
    this, PathVert(name), PathFrag(name)))


namespace GComponent {
    class MyGL;
    struct UniformVar {

    };


    class MyShader:public QOpenGLShaderProgram
    {
    public:
        /**
         * Shader 构造函数，此处封装了 QOpenGLShaderProgram
         * @param vertexPath   顶点着色器路径
         * @param fragmentPath 片段着色器路径
         * @param geometryPath 几何着色器路径
         * */
        MyShader(QObject * parent,const std::string& vertexPath, const std::string& fragmentPath, const std::string & geometryPath = "");

        std::shared_ptr<MyGL> gl;

        virtual ~MyShader();

        /// 调用 bind 直接使用 继承的 QOpenGLShaderProgram::bind() 效果相同
        void use() noexcept;

        void SetGL(std::shared_ptr<MyGL> other);

        ///
        inline void SetName(const std::string& name) { name_ = name; }

        /// Set 系列函数，使用前请先使用 MyShader::use
        void setBool(const std::string & name, bool value) noexcept;
        void setInt(const std::string & name, int value) noexcept;
        void setUint(const std::string& name, unsigned value) noexcept;
        void setFloat(const std::string& name, float value) noexcept;
        void setVec2(const std::string& name, float value[2]) noexcept;
        void setVec2(const std::string& name, glm::vec2 value) noexcept;
        void setVec4(const std::string& name, float value[4]) noexcept;
        void setVec3(const std::string& name, float value[3]) noexcept;
        void setVec3(const std::string& name, glm::vec3 value) noexcept;
        void setMat4(const std::string& name, glm::mat4 mat) noexcept;
    
    private:
        static std::string GetTpyeName(GLenum type);

    private:
        std::string name_;
        bool init_ = false;
    
    };

    
}


#endif // MYSHADER_H
