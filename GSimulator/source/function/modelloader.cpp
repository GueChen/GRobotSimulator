#include "modelloader.h"

#include "function/stringprocessor.hpp"

#include <array>
#include <unordered_map>
#include <algorithm>
#include <execution>
#include <sstream>
#include <iostream>
#include <filesystem>


using namespace GComponent;

std::tuple<std::vector<Vertex>, std::vector<Triangle>> ModelLoader::ReadFile(const std::string & file_path)
{
    size_t filter = file_path.find_last_of('.');
    auto lenth = file_path.length();
    auto file_type = file_path.substr(filter + 1, lenth - filter - 1);
    auto file_name = file_path.substr(0, filter);

    std::cout << "File Name: " << file_name << std::endl;
    std::cout << "File Type: " << file_type << std::endl;
    if (file_type == "ply")
    {
        return ReadPlyFile(file_path);
    }
    else if (file_type == "STL")
    {        
        return ReadSTLFile(file_path);
    }
    else if (file_type == "obj")
    {
        return ReadOBJFile(file_path);
    }
    else
    {
        return {};
    }
}

std::tuple<std::vector<Vertex>, std::vector<Triangle>> ModelLoader::ReadPlyFile(const std::string & file_path)
{
    
    std::string     content;
    std::ifstream   ply_file;                           // 文件路径初始化
    ply_file.exceptions(                                // 确保异常可抛出
        std::ifstream::failbit | std::ifstream::badbit);

    try {                                               // 打开文件
        ply_file.open(file_path);
        std::stringstream file_stream;
        file_stream << ply_file.rdbuf();
        ply_file.close();
        content = file_stream.str();
    }
    catch (std::ifstream::failure e)
    {
        std::cout << "ERROR::MODELLOADER::FILE_NOT_SUCCESS_READ:\n"
            "The file name is:" << file_path << std::endl;
    }   
    
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
   
    line = 0;
    while ((next = content.find('\n', offset)) != -1 && line < VertexLine)
    {
        auto partCont = content.substr(offset, next - offset);
        offset = ++next;

        auto vals = StringProcessor::Split(partCont);
        Positions.push_back(vec3(std::stof(vals[0]), std::stof(vals[1]), std::stof(vals[2])));
        line++;
    }
   
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
        vec3 _edge1 = vertices[Indices[i].third].position  - vertices[Indices[i].first].position;
        vec3 _edge2 = vertices[Indices[i].second].position - vertices[Indices[i].first].position;

        auto norm = glm::normalize(glm::cross(_edge1, _edge2));

        vertices[Indices[i].first].normal  += norm;
        vertices[Indices[i].second].normal += norm;
        vertices[Indices[i].third].normal  += norm;
    }

    // 法向量正则化 
    for (auto& vert : vertices)
    {
        vert.normal = glm::normalize(vert.normal);
    }

    return {vertices, Indices };

}

std::tuple<std::vector<Vertex>, std::vector<Triangle>> ModelLoader::ReadSTLFile(const std::string& filePath)
{
    static constexpr const size_t BinaryHeaderSize = 84;
    static constexpr const size_t TriangleSize     = 50;

    std::ifstream f;
    f.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    
    // open file
    try {
        f.open(filePath, std::ios_base::binary| std::ios_base::in);
    }
    catch (std::ifstream::failure e){
        std::cout << "ERROR::MODELLOADER::FILE_NOT_SUCCESS_READ:\n"
            "The file name is:" << filePath << std::endl;
    }

    // File Format Check Acording file size
    Encoding fmt = Encoding::ASCII;
    size_t file_size = std::filesystem::file_size(filePath);  
    if (file_size > BinaryHeaderSize) {
        f.seekg(BinaryHeaderSize - sizeof(int));                                    // move cursor to header over pos get triangle size

        int count = 0; f.read(reinterpret_cast<char*>(&count), sizeof count);       // read and check whether the 2 sizes are match (triangles and  file)
        if ((BinaryHeaderSize + count * TriangleSize) == file_size) {
            fmt = Encoding::Binary;            
        }
        f.seekg(0);
    }

    switch (fmt) {
    case Encoding::ASCII: {
        return ReadSTLAscii(f);
        break;
    }
    case Encoding::Binary: {
        return ReadSTLBinary(f);
        break;
    }
    default: {
        throw std::exception("Unsurpported fmt read, pls check whether ur file not broken\n");
        return {};
    }
    }

    
}

std::tuple<std::vector<Vertex>, std::vector<Triangle>> GComponent::ModelLoader::ReadOBJFile(const std::string& file_path)
{    
    using namespace tinyobj;    
    ObjReaderConfig config;  
    ObjReader       reader;

    config.mtl_search_path = "./";
    if (!reader.ParseFromFile(file_path, config)) {
        if (!reader.Error().empty()) {
            std::cerr << "OBJ_FILE_READ_ERROR: " << reader.Error();
            return {};
        }
    }

    if (!reader.Warning().empty())
    {
        std::cout << "OBJ_FILE_WARNING: " << reader.Warning();
    }

    const attrib_t&               attrib    = reader.GetAttrib();
    const std::vector<shape_t>    shapes    = reader.GetShapes();
    const std::vector<material_t> materials = reader.GetMaterials();
    
    std::vector<Vertex>   vertices;
    std::vector<Triangle> triangles;
    std::unordered_map<Vertex, uint32_t> vertices_map;
    uint32_t count = 0;
    for (auto& shape : shapes) 
    {
        size_t index_count = 0;
        vector<int> triangle(3, -1);
        for (auto& index : shape.mesh.indices)
        {
            Vertex      vertex;
            glm::vec3 & pos   = vertex.position,
                      & norm  = vertex.normal;
            glm::vec2 & coord = vertex.texcoords;
            /* Process Position */
            pos.x = attrib.vertices[3 * index.vertex_index + 0];
            pos.y = attrib.vertices[3 * index.vertex_index + 1];
            pos.z = attrib.vertices[3 * index.vertex_index + 2];
            /* Process Normal */
            if (attrib.normals.size() >= 0)
            {
                norm.x = attrib.normals[3 * index.normal_index + 0];
                norm.y = attrib.normals[3 * index.normal_index + 1];
                norm.z = attrib.normals[3 * index.normal_index + 2];
            }
            else
            {
                norm.x = norm.y = norm.z = 0.0f;
            }
            /* Process Texcoord UV */
            if (attrib.texcoords.size() >= 0)
            {
                coord.x = attrib.texcoords[2 * index.texcoord_index + 0];
                coord.y = attrib.texcoords[2 * index.texcoord_index + 1];
            }
            else
            {
                coord.x = coord.y = 0.0f;
            }

            /* Not Contain the Vertex */
            if (!vertices_map.count(vertex))
            {
                vertices_map.emplace(vertex, count);
                vertices.push_back(vertex);
                ++count;                
            }            
            triangle[index_count % 3] = vertices_map[vertex];

            if (index_count == 2)
            {
                triangles.emplace_back(triangle[0], triangle[1], triangle[2]);
                index_count = -1;
            }
            ++index_count;          
        }
    }

    return {vertices, triangles};
}

/*__________________________________PROTECTED METHODS_______________________________________________________*/
ModelLoader::_ModelInfo ModelLoader::ReadSTLAscii(std::ifstream& file_stream)
{

    std::vector<vec3>       normal;
    std::vector<vec3>       pos;
    std::vector<Triangle>   ind;
    if (file_stream.is_open())
    {
        std::string lineContent;
        int count = 0;
        while (lineContent != "endsolid")
        {
            std::getline(file_stream, lineContent);
            if (lineContent.find("facet") != -1)
            {
                auto vals = StringProcessor::Split(lineContent);
                vec3 norm = vec3(std::stod(vals[2]), std::stod(vals[3]), std::stod(vals[4]));
                
                // 为 Vertex 法线赋值
                normal.push_back(norm);
                normal.push_back(norm);
                normal.push_back(norm);

                // 为 Vertex 位置赋值
                do{
                    std::getline(file_stream, lineContent);
                    if (lineContent.find("vertex") != -1)
                    {
                        auto vals = StringProcessor::Split(lineContent);
                        pos.push_back(
                            /*0.001f **/ vec3(std::stod(vals[1]) , std::stod(vals[2]), std::stod(vals[3]))
                        );
                    }
                } while (lineContent.find("endfacet") == -1);
            }
            // 为面索引赋值
            ind.emplace_back(count, count + 1, count + 2);           

            count += 3;
        }
        file_stream.close();
    }

    // 填充顶点
    std::vector<Vertex> vertices(pos.size());
    std::transform(std::execution::par_unseq,
        pos.begin(), pos.end(), normal.begin(), vertices.begin(), [](auto& p, auto& norm) {
            return Vertex(0.001f * p, norm, vec2(0.0f));
        });
    /*auto itv = vertices.begin();
    auto itn = normal.begin();
    for (auto& p : pos)
    {
        (*itv++) = Vertex{ p, -(*itn++), vec2(0.0f) };
    }*/

    return {vertices, ind};
}

ModelLoader::_ModelInfo ModelLoader::ReadSTLBinary(std::ifstream& file_stream)
{
    vector<Vertex>      vertices;   // vertex array (output value)
    vector<Triangle>    indices;    // index  array (output value)

    if (file_stream.is_open()) {
        /* strided the header */       
        file_stream.seekg(80, std::ios_base::beg);      
        
		/* triangle size */
		int triangle_count = 0; file_stream.read(reinterpret_cast<char*>(&triangle_count), sizeof triangle_count);
	
        /* fill the vertex and triangle data */
		vertices.assign(triangle_count * 3,  Vertex(vec3(0.0f), vec3(0.0f), vec2(0.0f)));
        indices.assign(triangle_count,       Triangle{ 0, 0, 0 });
        
        
		for (int i = 0, count = 0; i < triangle_count; ++i, count += 3) {
            std::array<vec3, 4> vecs;                        
            file_stream.read(reinterpret_cast<char*>(vecs.data()), sizeof vecs);

			/* fill vertex */
            vertices[count + 0].normal = vertices[count + 1].normal = vertices[count + 2].normal
                                         = vecs[0];
            vertices[count + 0].position = 0.001f * vecs[1];
			vertices[count + 1].position = 0.001f * vecs[2];
			vertices[count + 2].position = 0.001f * vecs[3];
            
            /* fill index */
            indices[i]  = Triangle(count, count + 1, count + 2);
            
            /* stride 2 bytes */                                    
            char stride_buff[2]; file_stream.read(stride_buff, sizeof stride_buff);           
		}
        file_stream.close();
	}
    return {vertices, indices};
}
