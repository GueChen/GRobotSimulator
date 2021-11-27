QT       += core gui opengl openglwidgets quickwidgets quick
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 no_include_pwd


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
message(Project Location:  $$PWD)
!exists(OPGL_INC)
{
    OPGL_INC = $(OPGL_INCLUDE)
    message(Already Add OPGL_INCUDE_FILES:)
    message($(OPGL_INCLUDE))
    INCLUDEPATH = $(OPGL_INCLUDE)\
        $$PWD
}

!exists(OPGL_LIB)
{
    OPGL_LIB = $(OPGL_LIB)
    message(Already Add OPGL_LIB_FILES:)
    message($(OPGL_LIB))
    LIBS += -L$(OPGL_LIB)\
       -lopengl32\
       -lglfw3
}

SOURCES += \
    Component/Geometry/mesh.cpp \
    Component/Geometry/modelloader.cpp \
    Component/Object/Robot/dual_arm_platform.cpp \
    Component/Object/Robot/joint.cpp \
    Component/Object/Robot/kuka_iiwa_model.cpp \
    Component/Object/Robot/robot_body_model.cpp \
    Component/Object/basegrid.cpp \
    Component/Object/model.cpp \
    Component/Struct/Tree/stringtree.cpp \
    Component/Struct/Tree/treeitem.cpp \
    Component/myshader.cpp \
    UI/DockWidgetComponent/dualarmviewwidget.cpp \
    UI/DockWidgetComponent/modelviewwidget.cpp \
    UI/Item/modelmanagerview.cpp \
    UI/ModelView.cpp \
    UI/OpenGLWidget/DualArmView.cpp \
    UI/testopenglwidget.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    Component/Camera.hpp \
    Component/GStruct.hpp \
    Component/Geometry/Texture.hpp \
    Component/Geometry/Vertex.hpp \
    Component/Geometry/mesh.h \
    Component/Geometry/modelloader.h \
    Component/MyGL.hpp \
    Component/Object/Robot/dual_arm_platform.h \
    Component/Object/Robot/joint.h \
    Component/Object/Robot/kuka_iiwa_model.h \
    Component/Object/Robot/robot_body_model.h \
    Component/Object/basegrid.h \
    Component/Object/model.h \
    Component/Struct/Tree/stringtree.h \
    Component/Struct/Tree/treeitem.h \
    Component/myshader.h \
    UI/DockWidgetComponent/dualarmviewwidget.h \
    UI/DockWidgetComponent/modelviewwidget.h \
    UI/Item/modelmanagerview.h \
    UI/ModelView.h \
    UI/OpenGLWidget/DualArmView.h \
    UI/testopenglwidget.h \
    mainwindow.h

FORMS += \
    UI/DockWidgetComponent/dualarmviewwidget.ui \
    UI/DockWidgetComponent/modelviewwidget.ui \
    mainwindow.ui

EVERYTHING = $$SOURCES $$HEADERS
message("The Project contains following files:")
message($$EVERYTHING)

message("The Project contains following libs:")
message($$LIBS)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    Shader/Fragment/BaseFrag.frag \
    Shader/Fragment/ColorFrag.frag \
    Shader/Fragment/PhoneFrag.frag \
    Shader/Fragment/TriangleFrag.frag \
    Shader/Vertex/BaseVert.vert \
    Shader/Vertex/ColorVert.vert \
    Shader/Vertex/PhoneVert.vert \
    Shader/Vertex/TriangleVert.vert \
    UI/QML/BasicController.qml \
    UI/QML/CustomSlider.qml \
    UI/QML/MySlider.qml \
    UI/QML/SliderControl.qml \
    UI/QML/SliderControlForm.qml

