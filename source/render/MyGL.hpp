#ifndef _MYGL_H
#define _MYGL_H

#include <stb_image.h>

#include <QtOpenGL/QOpenGLFunctions_4_5_Core>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <concepts>
#include <iostream>
#include <format>
#include <vector>
#include <tuple>

namespace GCONST {
    constexpr size_t FLOAT_SIZE = sizeof(float);
    constexpr size_t INT_SIZE  = sizeof(int);
    constexpr size_t MAT4_SIZE = sizeof(glm::mat4);
    constexpr size_t VEC4_SIZE = sizeof(glm::vec4);
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

/*______________________________________OpenGL Helper METHODS_______________________________________*/
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

    unsigned
    genDirLightViewPos()
    {
        unsigned uboGlobalParameter;
        
        glGenBuffers(1, &uboGlobalParameter);

        glBindBuffer(GL_UNIFORM_BUFFER, uboGlobalParameter);
        glBufferData(GL_UNIFORM_BUFFER, 3 * GCONST::VEC4_SIZE, NULL, GL_STATIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, uboGlobalParameter);

        return uboGlobalParameter;
    }

    void
    setDirLightViewPos(unsigned ubo, glm::vec3 light_dir, glm::vec3 light_color, glm::vec3 view_pos)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, ubo);

        glBufferSubData(GL_UNIFORM_BUFFER, 0 * GCONST::VEC4_SIZE, GCONST::VEC3_SIZE, glm::value_ptr(light_dir));
        glBufferSubData(GL_UNIFORM_BUFFER, 1 * GCONST::VEC4_SIZE, GCONST::VEC3_SIZE, glm::value_ptr(light_color));
        glBufferSubData(GL_UNIFORM_BUFFER, 2 * GCONST::VEC4_SIZE, GCONST::VEC3_SIZE, glm::value_ptr(view_pos));
       
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

/*____________________________________IMAGE PROCESS Methods__________________________________________*/
    template<class T> requires std::same_as<T, std::string> || std::same_as<T, std::string_view>    
    unsigned
    LoadCubemap(const std::vector<T>& faces) 
    {
        unsigned texture_buffer = 0;
        glGenTextures(1, &texture_buffer);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture_buffer);
        int width, height, nr_channels;
        for (unsigned i = 0; i < faces.size(); ++i) 
        {
            unsigned char* data = stbi_load(faces[i].data(), &width, &height, &nr_channels, 0);
            if (data) 
            {
                GLenum format = GL_RGB;
                switch (nr_channels) 
                {
                case 1:format = GL_RED; break;
                case 3:format = GL_RGB; break;
                case 4:format = GL_RGBA; break;
                default: format = GL_RGB;
                }
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            }
            else 
            {
                std::cerr << std::format("Cubemap texture failed loading at path: {:}\n", faces[i]);
            }
            stbi_image_free(data);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        return texture_buffer;
    }

    unsigned
    LoadTexture(const std::string file_path, bool repeat = true)
    {
        unsigned int texture_buffer_handle = 0;
        glGenTextures(1, &texture_buffer_handle);

        int width, height, nr_channels;
        unsigned char* data = stbi_load(file_path.c_str(), &width, &height, &nr_channels, 0);
        if (data)
        {
            GLenum format       = GL_RGB;
            GLint  wrap_method  = repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE;
            switch (nr_channels)
            {
            case 1:format = GL_RED; break;
            case 3:format = GL_RGB; break;
            case 4:format = GL_RGBA; break;
            default: format = GL_RGB;
            }
            glBindTexture(GL_TEXTURE_2D, texture_buffer_handle);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
                 
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_method);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_method);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else
        {
            std::cerr << std::format("MyGL LoadTexture Error: {:}\n", file_path);            
        }
        return texture_buffer_handle;
    }

/*____________________________________SHADER Helper Methods__________________________________________*/
    void setBool(unsigned model_id_, const std::string & name, bool value) noexcept
    {
        glUniform1i(glGetUniformLocation(model_id_, name.c_str()), (int)value);
    }

    void setInt(unsigned model_id_, const std::string & name, int value) noexcept
    {
        glUniform1i(glGetUniformLocation(model_id_, name.c_str()), value);
    }

    void setFloat(unsigned model_id_, const std::string& name, float value) noexcept
    {
        glUniform1f(glGetUniformLocation(model_id_, name.c_str()), value);
    }

    void setVec2(unsigned model_id_, const std::string& name, float value[2]) noexcept
    {
        glUniform2fv(glGetUniformLocation(model_id_, name.c_str()), 1, value);
    }

    void setVec2(unsigned model_id_, const std::string& name, glm::vec2 value) noexcept
    {
        glUniform2fv(glGetUniformLocation(model_id_, name.c_str()), 1, value_ptr(value));
    }

    void setVec4(unsigned model_id_, const std::string& name, float value[4]) noexcept
    {
        glUniform4f(glGetUniformLocation(model_id_, name.c_str()), value[0], value[1], value[2], value[3]);
    }

    void setVec3(unsigned model_id_, const std::string& name, float value[3]) noexcept
    {
        glUniform3fv(glGetUniformLocation(model_id_, name.c_str()), 1, value);
    }

    void setVec3(unsigned model_id_, const std::string& name, glm::vec3 value) noexcept
    {
        glUniform3fv(glGetUniformLocation(model_id_, name.c_str()), 1, value_ptr(value));
    }

    void setMat4(unsigned model_id_, const std::string& name, glm::mat4 mat) noexcept
    {
        glUniformMatrix4fv(glGetUniformLocation(model_id_, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    }
};

}

#endif // MYGL_H
