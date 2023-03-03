# CMakeList.txt: PBR_PROJECT 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

project(GSimulatorEditor VERSION 0.0.3 LANGUAGES CXX)

add_definitions(-D_COLLISION_TEST)

if(WIN32)
	add_compile_options(/W4)
	add_compile_options(/wd4828)
endif()

set(TARGET_NAME GSimulatorEditor)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_PREFIX_PATH "${QT_DIR}")

file(GLOB_RECURSE HEADER_FILES "source/*.h" "source/*.hpp")
file(GLOB_RECURSE SOURCE_FILES "source/*.cpp" "source/*.c")
file(GLOB_RECURSE UI_FILES "UI/*.ui")
file(GLOB_RECURSE QRC_FILES "resource/*.qrc")

set(UI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/UI")
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_AUTOUIC_SEARCH_PATHS} 
	"${CMAKE_CURRENT_SOURCE_DIR}/UI"
	"${CMAKE_CURRENT_SOURCE_DIR}/UI/Dialog"
	"${CMAKE_CURRENT_SOURCE_DIR}/UI/Display"
	"${CMAKE_CURRENT_SOURCE_DIR}/UI/widget"
)

#___________________ Third Party Dependencies __________________________________________
find_package(Qt6 COMPONENTS				# QT Dependencies
	Gui 
	Core 
	Widgets  
	OpenGL
	Charts
	SerialPort
	Network
	REQUIRED
)
find_package(Stb REQUIRED)				# Stb
find_package(tinyobjloader REQUIRED)	# tinyobjloader
find_package(glad REQUIRED)				# sky box design problems
find_library(PHYSX_FOUNDATION PhysxFoundation_64  PATHS ${VCPKG_DYNAMIC_LIB})
find_library(PHYSX_COMMON     PhysXCommon_64	  PATHS ${VCPKG_DYNAMIC_LIB})
find_library(PHYSX_COOKING	  PhysXCooking_64     PATHS ${VCPKG_DYNAMIC_LIB})
find_library(PHYSX_PHYSICS	  PhysX_64			  PATHS ${VCPKG_DYNAMIC_LIB})
# static lib
find_library(PHYSX_STATIC_EXT PhysXExtensions_static_64 PATHS ${VCPKG_STATIC_LIB})
find_library(PHYSX_STATIC_PVD PhysXPvdSDK_static_64     PATHS ${VCPKG_STATIC_LIB})

#___________________ Display _____________________________________________________________
message("UI files	 = ${UI_FILES}")
message("Qt6 dir	 = ${Qt6_DIR}")
message("UI dir		 = ${UI_DIR}")
message("HEADER_FILES = ${HEADER_FILES}")
message("CMAKE_CUURENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")
#
get_target_property(_QMAKE_EXECUTABLE Qt6::qmake IMPRTED_LOCATION)
get_filename_component(_QT_BIN_DIR "${_QMAKE_EXECUTABLE}" DIRECTORY)
find_program(DEPLOYQT_EXECUTABLE NAMES windeployqt macdeployqt "${_QT_BIN_DIR}")

#___________________ Generate Target And Properties ______________________________________
add_executable (${TARGET_NAME} 
	${UI_FILES} 
	${SOURCE_FILES} 
	${HEADER_FILES}
	${QRC_FILES}
)

if(WIN32)
	add_custom_command(TARGET ${TARGET_NAME} 
        POST_BUILD
        COMMAND ${DEPLOYQT_EXECUTABLE} "$<TARGET_FILE:${TARGET_NAME}>"
            --$<LOWER_CASE:$<CONFIG>> --qmldir ${CMAKE_SOURCE_DIR}
    )
endif()
#_______ Copy File to Bin __________________________________________________
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD 
		COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/asset" "$<TARGET_FILE_DIR:${TARGET_NAME}>/asset"
		COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Shader" "$<TARGET_FILE_DIR:${TARGET_NAME}>/Shader"
)

set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${TARGET_NAME} PROPERTY LINKER_LANGUAGE CXX)

#
target_include_directories(
	${TARGET_NAME} PUBLIC		
	./source
)

#___________________ Linking Libraries ______________________
target_link_libraries(${TARGET_NAME} PUBLIC 
	common_lib
	Qt6::Core
	Qt6::Widgets
	Qt6::Gui
	Qt6::OpenGL
	Qt6::Charts
	Qt6::SerialPort
	Qt6::Network
	tinyobjloader::tinyobjloader
	glad::glad
	${PHYSX_FOUNDATION}
	${PHYSX_COMMON}
	${PHYSX_COOKING}
	${PHYSX_PHYSICS}
	${PHYSX_STATIC_EXT}
	${PHYSX_STATIC_PVD}
)

