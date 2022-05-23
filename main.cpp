#include "mainwindow.h"
#include "model/robot/dual_arm_platform.h"
#include "engineapp.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    
    GComponent::EngineApp::getInstance().Init(argc, argv);

    GComponent::DUAL_ARM_PLATFORM robot;
     
    GComponent::EngineApp::getInstance().Exec();

    return 0;
}
