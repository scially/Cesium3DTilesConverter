cmake_minimum_required (VERSION 3.8)
project(3dtiles)

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
	add_definitions(-DNOMINMAX)
endif()

find_package(GDAL REQUIRED)
find_package(Qt5 
	REQUIRED
	COMPONENTS Core Xml Gui Test)

find_package(OpenSceneGraph 
	REQUIRED 
	COMPONENTS osgDB osgGA osgUtil osgViewer)
message(status ${OPENSCENEGRAPH_LIBRARIES})	
message(status ${OPENSCENEGRAPH_INCLUDE_DIRS})

# 将源代码添加到此项目的可执行文件。
include_directories(
	"include"
	${OPENSCENEGRAPH_INCLUDE_DIRS} 
	${GDAL_INCLUDE_DIR})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_subdirectory(src)

enable_testing(true)
add_subdirectory(test)