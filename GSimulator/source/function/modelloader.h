/**
 *  @file  	modelloader.h
 *  @brief 	This class supplies interface to transfer file to Vertex and Triangle.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Oct   25, 2021 
 *  @update April 11, 2022 modifier the type to Triangle to adpater interface
 *          May   12, 2022 add .obj file using tinyobjloader and seperate Mesh to a adapter class
 **/
#ifndef MODELLOADER_H
#define MODELLOADER_H

#include "render/rendering_datastructure.hpp"

#include <tiny_obj_loader.h>

#include <string>
#include <vector>
#include <fstream>

namespace GComponent {

using std::tuple;
using std::vector;
using std::string;

class ModelLoader
{
    using _ModelInfo = tuple<vector<Vertex>, vector<Triangle>>;
public:
    // file encoding format
    enum class Encoding {
        ASCII   = 0,
        Binary  = 1
    };
public:
    ModelLoader() = delete;
    
    /// <summary>
    /// Load model from file acording to file format
    /// <para>
    /// 根据文件类型从文件中获取模型
    /// </para>
    /// </summary>
    /// <param name="file_path">    cref {string}               [in]  file_path                                     文件路径    </param>
    /// <returns>                        {Vertexs, Triangles}   [out] arrays tuple of vertex and triangle index     顶点数组    </returns>
    static _ModelInfo ReadFile(const string& file_path);

    /// <summary>
    /// Load model from ply-file
    /// <para>
    /// 从 ply 文件中读取模型
    /// </para>
    /// </summary>
    /// <param name="file_path">    cref {string}               [in]  file_path                                     文件路径    </param>
    /// <returns>                        {Vertexs, Triangles}   [out] arrays tuple of vertex and triangle index     顶点数组    </returns>
    static _ModelInfo ReadPlyFile(const string& file_path);

    /// <summary>
    /// Load model from stl-file
    /// <para>
    /// 从 STL 文件中读取模型
    /// </para>
    /// </summary>
    /// <param name="file_path">    cref {string}               [in]  file_path                                     文件路径    </param>
    /// <returns>                        {Vertexs, Triangles}   [out] arrays tuple of vertex and triangle index     顶点数组    </returns>
    static _ModelInfo ReadSTLFile(const string& file_path);

    /// <summary>
    /// Load model from obj-file
    /// <para>
    /// 从 obj 文件中读取模型
    /// </para>
    /// </summary>
    /// <param name="file_path">    cref {string}               [in]  file_path                                     文件路径    </param>
    /// <returns>                        {Vertexs, Triangles}   [out] arrays tuple of vertex and triangle index     顶点数组    </returns>
    static _ModelInfo ReadOBJFile(const string& file_path);

protected:
    static _ModelInfo ReadSTLAscii(std::ifstream& file_stream);
    static _ModelInfo ReadSTLBinary(std::ifstream& file_stream);
};

}


#endif // MODELLOADER_H
