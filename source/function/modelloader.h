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

namespace GComponent {
    
class ModelLoader
{
public:
    ModelLoader() = delete;
    static std::tuple<std::vector<Vertex>, std::vector<Triangle>> readFile(const std::string& file_path);
    static std::tuple<std::vector<Vertex>, std::vector<Triangle>> readPlyFile(const std::string& file_path);
    static std::tuple<std::vector<Vertex>, std::vector<Triangle>> readSTLFile(const std::string& file_path);
    static std::tuple<std::vector<Vertex>, std::vector<Triangle>> readOBJFile(const std::string& file_path);
    static std::string getFileContent(const std::string& file_path);
    static std::vector<float> vStringToFloat(const std::vector<std::string> & vs);

};

}


#endif // MODELLOADER_H
