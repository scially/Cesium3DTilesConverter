cmake_minimum_required (VERSION 3.10)
project(Cesium3DTilesConverter VERSION 3.1.0)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

configure_file (
    "${PROJECT_SOURCE_DIR}/include/Commons/Version.h.in"
    "${PROJECT_SOURCE_DIR}/include/Commons/Version.h"
)

find_package(GDAL REQUIRED)
find_package(Qt5 
	REQUIRED
	COMPONENTS Core Xml Concurrent Test)
find_package(OpenSceneGraph 
	REQUIRED 
	COMPONENTS osg osgDB osgUtil)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_subdirectory(thirdparty)
add_subdirectory(src)

enable_testing(TRUE)
add_subdirectory(tests)
