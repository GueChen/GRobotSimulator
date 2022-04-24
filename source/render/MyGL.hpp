//TODO: 补充工具函数注释或文档
#ifndef MYGL_H
#define MYGL_H

#include <QtOpenGL/QOpenGLFunctions_4_5_Core>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <tuple>

// TODO:考虑是否把该宏移入 MyShader中
#define PathVert(name) ("./Shader/Vertex/"#name"Vert.vert")
#define PathFrag(name) ("./Shader/Fragment/"#name"Frag.frag")

namespace GCONST {
    constexpr size_t FLOAT_SIZE = sizeof(float);
    constexpr size_t INT_SIZE = sizeof(int);
    constexpr size_t MAT4_SIZE = sizeof(glm::mat4);
    constexpr size_t VEC3_SIZE = sizeof(glm::vec3);
    constexpr size_t VEC2_SIZE = sizeof(glm::vec2);
}

namespace GComponent{

using std::pair;
using std::tuple;

class MyGL:public QOpenGLFunctions_4_5_Core{
public:
    MyGL()  = default;
    ~MyGL() = default;

    unsigned
    genMatrices()
    {
        unsigned uboMatrices;

        glGenBuffers(1, &uboMatrices);

        glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
        glBufferData(GL_UNIFORM_BUFFER, 2 * GCONST::MAT4_SIZE, NULL, GL_STATIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboMatrices);

        return uboMatrices;
    }

    void
    setMatrices(unsigned ubo, glm::mat4 pro, glm::mat4 view)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, ubo);

        glBufferSubData(GL_UNIFORM_BUFFER,
                        0, GCONST::MAT4_SIZE,
                        glm::value_ptr(pro));
        glBufferSubData(GL_UNIFORM_BUFFER,
                        GCONST::MAT4_SIZE, GCONST::MAT4_SIZE,
                        glm::value_ptr(view));

        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    pair<unsigned, unsigned>
    genVABO(void* data, size_t size)
    {
        unsigned VAO, VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
        return { VAO, VBO };
    }


    template<class... Args>
    unsigned genEBO(Args &&... _args)
    {
        static_assert(std::conjunction_v<std::_Is_specialization<std::remove_reference_t<Args>, std::vector>...>,
            "The Input Type can't cast Vector, ERROR::myGLTool::genEBO");
        unsigned EBO;
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        size_t size = ((sizeof(std::remove_reference_t<Args>::value_type) * _args.size()) +...);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);

        size_t offset = 0, tempSize = 0;
        ((  tempSize = sizeof(std::remove_reference_t<Args>::value_type) * _args.size(),
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, tempSize, &_args[0]),
            offset += tempSize),
            ...);

        return EBO;
    }

    template<class... Args>
    void EnableVertexAttribArrays(Args... _args)
    {
        static_assert(std::conjunction_v<std::is_same<int, Args>...>,
            "The Input Type is not Int, ERROR::myGLTool::EnableVertexAttribArrays");
        const unsigned TOTAL = (_args + ...);

        unsigned idx = 0, loc = 0;
        ((glEnableVertexAttribArray(idx),
          glVertexAttribPointer(
                            idx++,
                            _args,
                            GL_FLOAT,
                            GL_FALSE,
                            TOTAL * GCONST::FLOAT_SIZE,
                            (void*)(loc * GCONST::FLOAT_SIZE)),
          loc += _args),
          ...);

    }


    template<class...Args>
    void EnableVertexAttribArrays_continus(Args&& ... _args)
    {
        static_assert(std::conjunction_v<std::_Is_specialization<std::remove_reference_t<Args>, std::vector>...>,
            "The Input Type can't cast Vector, ERROR::myGLTool::EnableVertexAttribArrays_continus");

        unsigned idx = 0, stride = 0;
        size_t singleSize = 0, loc = 0;

        ((  singleSize = sizeof(std::remove_reference_t<Args>::value_type),
            stride     = singleSize/ sizeof(float),
            glEnableVertexAttribArray(idx),
            glVertexAttribPointer(idx++, stride, GL_FLOAT, GL_FALSE, singleSize, (void*)(loc)),
            loc += singleSize * _args.size()), ...);

    }

    void setBool(unsigned ID, const std::string & name, bool value) noexcept
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }

    void setInt(unsigned ID, const std::string & name, int value) noexcept
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setFloat(unsigned ID, const std::string& name, float value) noexcept
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setVec2(unsigned ID, const std::string& name, float value[2]) noexcept
    {
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, value);
    }

    void setVec2(unsigned ID, const std::string& name, glm::vec2 value) noexcept
    {
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, value_ptr(value));
    }

    void setVec4(unsigned ID, const std::string& name, float value[4]) noexcept
    {
        glUniform4f(glGetUniformLocation(ID, name.c_str()), value[0], value[1], value[2], value[3]);
    }

    void setVec3(unsigned ID, const std::string& name, float value[3]) noexcept
    {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, value);
    }

    void setVec3(unsigned ID, const std::string& name, glm::vec3 value) noexcept
    {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, value_ptr(value));
    }

    void setMat4(unsigned ID, const std::string& name, glm::mat4 mat) noexcept
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    }
};

}

#endif // MYGL_H
