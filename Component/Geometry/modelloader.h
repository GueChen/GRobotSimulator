#ifndef MODELLOADER_H
#define MODELLOADER_H
#include <string>
#include <Component/Geometry/mesh.h>

namespace GComponent {
    
    class ModelLoader
    {
    public:
        ModelLoader() = delete;
        static std::tuple<std::vector<Vertex>, std::vector<unsigned>> readFile(const std::string & filePath);
        static std::tuple<std::vector<Vertex>, std::vector<unsigned>> readPlyFile(const std::string & filePath);
        static std::tuple<std::vector<Vertex>, std::vector<unsigned>> readSTLFile(const std::string& filePath);
        static std::string getFileContent(const std::string& filePath);
        static std::vector<std::string> Split(const std::string& s, char sparse = ' ');
        static std::vector<float> vStringToFloat(const std::vector<std::string> & vs);
    };

}


#endif // MODELLOADER_H
