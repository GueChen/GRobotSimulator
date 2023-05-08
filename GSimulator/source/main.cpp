#include "ui/mainwindow.h"
#include "engineapp.h"

#include <QtWidgets/QApplication>
// TODO: add Serializer and Deserializer then Delete Later
#include <glm/glm.hpp>
#include <map>

int main(int argc, char *argv[])
{
    using namespace GComponent;
    GComponent::EngineApp::getInstance().Init(argc, argv);  
    GComponent::EngineApp::getInstance().Exec();
    
    return 0;
}
