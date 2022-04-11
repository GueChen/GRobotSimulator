QT       +=  core gui opengl openglwidgets quickwidgets quick charts
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++latest

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
message(Project Location:  $$PWD)
!exists(OPGL_INC || OPGL_LIB)
{
    OPGL_INC = $(OPGL_INCLUDE)
    OPGL_LIB = $(OPGL_LIB)
    INCLUDEPATH =       \
        $(OPGL_INCLUDE) \
        $$PWD

    LIBS +=      \
    -L$(OPGL_LIB)\
       -lopengl32\
       -lglfw3
}

!exists(GInclude)
{
    GInclude = $(MyCppLib)
    ## $(GInclude) 失效原因未知
    INCLUDEPATH +=                  \
        "E:\lib\Cpp\MyLib"          \
        $(QT_INCLUDE)

}

!exists(EIGEN)
{
    EIGEN = $(VCPKG_INSTALL_x64)\include
    message("COMPARE:")
    message($$EIGEN)
    message("D:\programming\Tool\vcpkg\vcpkg\installed\x64-windows\include")
    ##INCLUDEPATH += $$EIGEN
    INCLUDEPATH += "D:\programming\Tool\vcpkg\vcpkg\installed\x64-windows\include"
}



SOURCES += \
    Base/editortreemodel.cpp \
    Base/treeitem.cpp \
    Component/Geometry/mesh.cpp \
    Component/Geometry/modelloader.cpp \
    Component/Object/BasicMesh/Gball.cpp \
    Component/Object/BasicMesh/gcurves.cpp \
    Component/Object/BasicMesh/gline.cpp \
    Component/Object/Motion/circmotion.cpp \
    Component/Object/Motion/linmotion.cpp \
    Component/Object/Motion/ptpmotion.cpp \
    Component/Object/Motion/splinemotion.cpp \
    Component/Object/Motion/syncduallinemotion.cpp \
    Component/Object/Robot/dual_arm_platform.cpp \
    Component/Object/Robot/joint.cpp \
    Component/Object/Robot/kuka_iiwa_model.cpp \
    Component/Object/Robot/robot_body_model.cpp \
    Component/Object/basegrid.cpp \
    Component/Object/gmotionbase.cpp \
    Component/Object/model.cpp \
    Component/myshader.cpp \
    Tooler/conversion.cpp \
    UI/Dialog/PathPlanningSettingDialog.cpp \
    UI/DockWidgetComponent/dualarmviewwidget.cpp \
    UI/DockWidgetComponent/modelviewwidget.cpp \
    UI/Item/modelmanagerview.cpp \
    UI/ModelView.cpp \
    UI/NormalWidget/buttongroup.cpp \
    UI/NormalWidget/datadisplaycanvas.cpp \
    UI/NormalWidget/slidercontroller.cpp \
    UI/OpenGLWidget/DualArmView.cpp \
    UI/testopenglwidget.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS +=\
    Base/editortreemodel.h \
    Base/treeitem.h \
    Component/Camera.hpp \
    Component/GStruct.hpp \
    Component/Geometry/Texture.hpp \
    Component/Geometry/Vertex.hpp \
    Component/Geometry/mesh.h \
    Component/Geometry/modelloader.h \
    Component/MyGL.hpp \
    Component/Object/BasicMesh/GBasicMesh \
    Component/Object/BasicMesh/Gball.h \
    Component/Object/BasicMesh/SimplexModel.hpp \
    Component/Object/BasicMesh/gcurves.h \
    Component/Object/BasicMesh/gline.h \
    Component/Object/Motion/GMotion \
    Component/Object/Motion/circmotion.h \
    Component/Object/Motion/linmotion.h \
    Component/Object/Motion/ptpmotion.h \
    Component/Object/Motion/splinemotion.h \
    Component/Object/Motion/syncduallinemotion.h \
    Component/Object/Robot/dual_arm_platform.h \
    Component/Object/Robot/joint.h \
    Component/Object/Robot/kuka_iiwa_model.h \
    Component/Object/Robot/robot_body_model.h \
    Component/Object/basegrid.h \
    Component/Object/gmotionbase.h \
    Component/Object/model.h \
    Component/myshader.h \
    Tooler/conversion.h \
    UI/Dialog/PathPlanningSettingDialog.h \
    UI/DockWidgetComponent/dualarmviewwidget.h \
    UI/DockWidgetComponent/modelviewwidget.h \
    UI/Item/modelmanagerview.h \
    UI/ModelView.h \
    UI/NormalWidget/buttongroup.h \
    UI/NormalWidget/datadisplaycanvas.h \
    UI/NormalWidget/slidercontroller.h \
    UI/OpenGLWidget/DualArmView.h \
    UI/testopenglwidget.h \
    mainwindow.h

FORMS += \
    UI/Dialog/PathPlanningSettingDialog.ui \
    UI/DockWidgetComponent/dualarmviewwidget.ui \
    UI/DockWidgetComponent/modelviewwidget.ui \
    UI/NormalWidget/buttongroup.ui \
    UI/NormalWidget/datadisplaycanvas.ui \
    UI/NormalWidget/slidercontroller.ui \
    mainwindow.ui

EVERYTHING = $$SOURCES $$HEADERS
message("The Project contains following incPath:")
message($$INCLUDEPATH)

message("The Project contains following libs:")
message($$LIBS)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    Shader/Fragment/BaseFrag.frag \
    Shader/Fragment/ColorFrag.frag \
    Shader/Fragment/LineColorFrag.frag \
    Shader/Fragment/PhoneFrag.frag \
    Shader/Fragment/TriangleFrag.frag \
    Shader/Vertex/BaseVert.vert \
    Shader/Vertex/ColorVert.vert \
    Shader/Vertex/LineColorVert.vert \
    Shader/Vertex/PhoneVert.vert \
    Shader/Vertex/TriangleVert.vert \
    UI/QML/BasicController.qml \
    UI/QML/CustomSlider.qml \
    UI/QML/MySlider.qml \
    UI/QML/SliderControl.qml \
    UI/QML/SliderControlForm.qml

