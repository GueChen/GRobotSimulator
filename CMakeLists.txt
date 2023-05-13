cmake_minimum_required (VERSION 3.8)

# configure cmake toolchain => vcpkg
if(DEFINED ENV{VCPKG})
	message("vcpkg found install path = $ENV{VCPKG}")
	set(VCPKG_ROOT "$ENV{VCPKG}\\scripts\\buildsystems\\vcpkg.cmake")
	set(CMAKE_TOOLCHAIN_FILE ${VCPKG_ROOT})
	set(VCPKG_DYNAMIC_LIB $ENV{VCPKG}/installed/${VCPKG_TARGET_TRIPLET}/bin)
	set(VCPKG_STATIC_LIB  $ENV{VCPKG}/installed/${VCPKG_TARGET_TRIPLET}/lib)
	set(CMAKE_PREFIX_PATH "$ENV{VCPKG}\\installed")
	message(STATUS "prefix path ${CMAKE_PREFIX_PATH}")
	message(STATUS "${CMAKE_TOOLCHAIN_FILE}")
else()
	message(FATAL_ERROR 
			"vcpkg not exist, please set it to environment variables\n"
			"format - VCPKG = <path>\n"
			"Windows Powershell user:\n"
			"$Env:VCPKG = '{vcpkg install path}'"
	)
endif()

if(WIN32)
	if(DEFINED ENV{QT_DIR})
		message(STATUS "QTDIR = $ENV{QT_DIR}")
		set(QT_DIR "$ENV{QT_DIR}")
	else()		
		message(FATAL_ERROR
			"QT_DIR not exist, please set it to environment variables\n"
			"format - QT_DIR = <path>\n"
			"Windows Powershell user:\n"
			"$Env:QT_DIR = '{qt installed path}'"
		)		
	endif()
endif()

project ("GSimulator")

# 包含子项目。
add_subdirectory ("3rdparty")
add_subdirectory ("GSimulator")
add_subdirectory ("GComponentBaseLib")
add_subdirectory ("GComponentQtPluginLib")
