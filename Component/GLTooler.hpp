/// 此头文件为对 gl 的 BufferManager 封装
/// Header Only
/// 当前为不考虑跨平台，或向不兼容的版本
/// 若要使得版本兼容则使用 QOpenGLEtraFunctions
/// 另此处依赖于两个第三方库：
/// ① QOpenGLFunctions
/// ② glm
/// 第二个库后续为整合物理仿真可能有待考虑改进
///

#ifndef GLTOOLER_HPP
#define GLTOOLER_HPP

#include <QOpenGLFunctions_4_5_Core>
#include<glm/glm.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<vector>
#include<tuple>
#include <QDebug>

#define PathVert(name) ("./Shader/Vertex/"#name"Vert.vert")
#define PathFrag(name) ("./Shader/Fragment/"#name"Frag.frag")

namespace GCONST {
    constexpr size_t FLOAT_SIZE = sizeof(float);
    constexpr size_t INT_SIZE = sizeof(int);
    constexpr size_t MAT4_SIZE = sizeof(glm::mat4);
    constexpr size_t VEC3_SIZE = sizeof(glm::vec3);
}

namespace GComponent{
    using std::pair;
    using std::tuple;
    class MyGL:public QOpenGLFunctions_4_5_Core{
    public:
        MyGL() = default;
        ~MyGL() = default;
        /**
         * 生成矩阵接口块缓存，适用于具有特殊布局的情况
         * @param {void} 仅生成矩阵块位置
         * @return {unsigned} 返回的内存块句柄，指向接口块的内部输入位置
         * */
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
        /**
         * 将数据绑定在两个内存块上，传入数组，用于绑定并返回VertexArray
         * @param data {float[]} 数组，用于传递需传给Shader的数组
         * @param size {size_t} 大小，用于传递数据的大小，防止越界
         * @return {pair<u, u>} 返回句柄， 用于返回顶点位置以及顶点数据缓存的位置
         */
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

        /**
         * 将顶点连接序号按，顺序打出，用途由自己定义返回BufferIdx
         * @param _args {vector<T>}  用于传递不同的顶点连接方式
         * @return {unsigned} 返回句柄，返回该内存块的位置
         */
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

        /**
         * 用于激活传递数据指针， 使数据能够传递到正确的指针位置
         * 排布示意:[ loc0|norm0|coord0
         *          loc1|norm1|coord1 ...]
         * 参数使用方法:
         *          args0,args1,args2
         * @tparam Args 参数包 此处仅适用int
         * @param _args 激活指针的顺序参数，用于定义传递的内存排布
         * */
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


        /**
         * 用于激活传递数据指针，使数据能够传递到正确的指针位置，用于相同数据连续排布情况<p>
         * 排布示意：[ locs|norms|coords]
         * @tparam Args 参数包 此处仅适用vector的特化
         * @param _args 激活指针的顺序参数，用于定义传递的内存排布
         */
        template<class...Args>
        void EnableVertexAttribArrays_continus(Args&& ... _args)
        {
            static_assert(std::conjunction_v<std::_Is_specialization<std::remove_reference_t<Args>, std::vector>...>,
                "The Input Type can't cast Vector, ERROR::myGLTool::EnableVertexAttribArrays_continus");

            unsigned idx = 0, stride = 0;
            size_t singleSize = 0, loc = 0;

            ((  singleSize = sizeof(std::remove_reference_t<Args>::value_type),
                stride =  singleSize/ sizeof(float),
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

#endif // GLTOOLER_HPP
