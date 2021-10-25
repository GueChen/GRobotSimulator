QT       += core gui opengl openglwidgets
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
    Component/Object/basegrid.cpp \
    Component/Object/model.cpp \
    Component/Struct/Tree/stringtree.cpp \
    Component/Struct/Tree/treeitem.cpp \
    Component/myshader.cpp \
    UI/Item/modelmanagerview.cpp \
    UI/OpenGLWidget/modelview.cpp \
    UI/animdockwidget.cpp \
    UI/modelviewdockwidget.cpp \
    UI/myopglwidget.cpp \
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
    Component/Object/basegrid.h \
    Component/Object/model.h \
    Component/Struct/Tree/stringtree.h \
    Component/Struct/Tree/treeitem.h \
    Component/myshader.h \
    UI/Item/modelmanagerview.h \
    UI/OpenGLWidget/modelview.h \
    UI/animdockwidget.h \
    UI/modelviewdockwidget.h \
    UI/myopglwidget.h \
    UI/testopenglwidget.h \
    mainwindow.h

FORMS += \
    UI/animdockwidget.ui \
    UI/modelviewdockwidget.ui \
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
    Shader/Vertex/TriangleVert.vert

