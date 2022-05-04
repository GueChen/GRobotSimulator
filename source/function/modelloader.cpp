#include "modelloader.h"

#include "function/stringprocessor.hpp"

#include <iostream>
#include <fstream>
#include <sstream>


using namespace GComponent;


std::tuple<std::vector<Vertex>, std::vector<Triangle>> ModelLoader::readFile(const std::string & filePath)
{
    size_t filter = filePath.find_last_of('.');
    auto lenth = filePath.length();
    auto fileType = filePath.substr(filter + 1, lenth - filter - 1);
    auto fileName = filePath.substr(0, filter);

    std::cout << "File Name: " << fileName << std::endl;
    std::cout << "File Type: " << fileType << std::endl;
    if (fileType == "ply")
    {
        return readPlyFile(filePath);
    }
    if (fileType == "STL")
    {
        std::cout << " is STL" << std::endl;
        return readSTLFile(filePath);
    }
    
}

std::tuple<std::vector<Vertex>, std::vector<Triangle>> ModelLoader::readPlyFile(const std::string & filePath)
{
    
    auto content = getFileContent(filePath);
    
    size_t offset = 0, next = 0;
    int VertexLine = 0, FaceLine = 0;
    int line = 0;
    while ((next = content.find('\n', offset)) != -1)
    {
        auto partCont = content.substr(offset, next - offset);
        offset = ++next;
        if (partCont == "end_header")
        {
            break;
        }

        if (partCont.find("element") != -1)
        {
            if (partCont.find("vertex") != -1)
            {
                auto pos = partCont.find_last_of(' ');
                VertexLine = std::stoi(partCont.substr(pos + 1, partCont.length() - pos + 1));
            }
            if (partCont.find("face") != -1)
            {
                auto pos = partCont.find_last_of(' ');
                FaceLine = std::stoi(partCont.substr(pos + 1, partCont.length() - pos + 1));
            }
        }
        ++line;
    }
    std::vector<vec3> Positions;
    std::vector<Triangle> Indices;
    std::cout << "#Vertex Lines = " << VertexLine << std::endl;
    std::cout << "#Face Lines = " << FaceLine << std::endl;
    line = 0;
    while ((next = content.find('\n', offset)) != -1 && line < VertexLine)
    {
        auto partCont = content.substr(offset, next - offset);
        offset = ++next;

        auto vals = StringProcessor::Split(partCont);
        Positions.push_back(vec3(std::stof(vals[0]), std::stof(vals[1]), std::stof(vals[2])));
        line++;
    }
    std::cout << "Vertex Line Over" << std::endl;
    line = 0;
    while ((next = content.find('\n', offset)) != -1 && line < FaceLine)
    {
        auto partCont = content.substr(offset, next - offset);
        offset = ++next;

        auto && vals = StringProcessor::Split(partCont);
        Indices.emplace_back(std::stoi(vals[1]), std::stoi(vals[2]), std::stoi(vals[3]));
 
        ++line;
    }
    
    // 填充顶点
    std::vector<Vertex> vertices(Positions.size());
    auto it = vertices.begin();
    for (auto& p : Positions)
    {
        (*it++) = Vertex{p, vec3(0.0f), vec2(0.0f)};
    }

    // 填充法向量
    for (int i = 0; i < Indices.size(); i += 3)
    {
        vec3 _edge1 = vertices[Indices[i].third].Position  - vertices[Indices[i].first].Position;
        vec3 _edge2 = vertices[Indices[i].second].Position - vertices[Indices[i].first].Position;

        auto norm = glm::normalize(glm::cross(_edge1, _edge2));

        vertices[Indices[i].first].Normal  += norm;
        vertices[Indices[i].second].Normal += norm;
        vertices[Indices[i].third].Normal  += norm;
    }


    // 法向量正则化 
    for (auto& vert : vertices)
    {
        vert.Normal = glm::normalize(vert.Normal);
    }

    return {vertices, Indices };

}

std::tuple<std::vector<Vertex>, std::vector<Triangle>> ModelLoader::readSTLFile(const std::string& filePath)
{
    // auto content = getFileContent(filePath);
    std::ifstream f;
    f.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    
    try {
        f.open(filePath);
    }catch (std::ifstream::failure e)
    {
        std::cout << "ERROR::MODELLOADER::FILE_NOT_SUCCESS_READ:\n"
            "The file name is:" << filePath << std::endl;
    }
    std::vector<vec3> normal;
    std::vector<vec3> pos;
    std::vector<Triangle> ind;
    if (f.is_open())
    {
        std::string lineContent;
        int count = 0;
        while (lineContent != "endsolid")
        {
            std::getline(f, lineContent);
            if (lineContent.find("facet") != -1)
            {
                auto vals = StringProcessor::Split(lineContent);
                vec3 norm = vec3(std::stof(vals[2]), std::stof(vals[3]), std::stof(vals[4]));
                
                // 为 Vertex 法线赋值
                normal.push_back(norm);
                normal.push_back(norm);
                normal.push_back(norm);

                // 为 Vertex 位置赋值
                do{
                    std::getline(f, lineContent);
                    if (lineContent.find("vertex") != -1)
                    {
                        auto vals = StringProcessor::Split(lineContent);
                        pos.push_back(
                            0.001f * vec3(std::stof(vals[1]) , std::stof(vals[2]), std::stof(vals[3]))
                        );
                    }
                } while (lineContent.find("endfacet") == -1);
            }
            // 为面索引赋值
            ind.emplace_back(count, count + 1, count + 2);           

            count += 3;
            //std::cout << lineContent << std::endl;
        }
        f.close();
    }
    // 填充顶点
    std::vector<Vertex> vertices(pos.size());
    auto itv = vertices.begin();
    auto itn = normal.begin();
    for (auto& p : pos)
    {
        (*itv++) = Vertex{ p, -(*itn++), vec2(0.0f) };
    }

    return {vertices, ind};
}

MeshComponent ModelLoader::getMesh(const std::string &resource)
{
    auto && [Vs, Is] = ModelLoader::readFile(resource);
    MeshComponent m(Vs, Is, std::vector<Texture>{});
    return m;
}

std::string ModelLoader::getFileContent(const std::string& filePath)
{
    std::string content;
    std::ifstream plyFile;                          // 文件路径初始化
    plyFile.exceptions(                             // 确保异常可抛出
        std::ifstream::failbit |
        std::ifstream::badbit);
    try {
        plyFile.open(filePath);                     // 打开文件
        std::stringstream fileStream;
        fileStream << plyFile.rdbuf();
        plyFile.close();
        content = fileStream.str();
    }
    catch (std::ifstream::failure e)
    {
        std::cout << "ERROR::MODELLOADER::FILE_NOT_SUCCESS_READ:\n"
            "The file name is:" << filePath << std::endl;
    }
    return content;
}

std::vector<float> ModelLoader::vStringToFloat(const std::vector<std::string> & vs)
{
    std::vector<float> rvals(vs.size());
    int count = 0;
    for (auto& s : vs)
    {
        rvals[count++] = std::stof(s);
    }
    return rvals;
}

