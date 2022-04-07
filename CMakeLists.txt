cmake_minimum_required (VERSION 3.8)
project(Cesium3DTilesConverter)

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
	add_definitions(-DNOMINMAX)
endif()

option(ENABLE_TEST "Enable test" ON)

find_package(GDAL REQUIRED)
find_package(PROJ REQUIRED)
find_package(Qt5 
	REQUIRED
	COMPONENTS Core Xml Test)

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

if(ENABLE_TEST)
	enable_testing(true)
	add_subdirectory(test)
	add_custom_target(CopyResources ALL
			COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/proj_data ${PROJECT_BINARY_DIR}/src/proj_data
			COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/proj_data ${PROJECT_BINARY_DIR}/test/proj_data
			COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/gdal_data ${PROJECT_BINARY_DIR}/src/gdal_data
			COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/gdal_data ${PROJECT_BINARY_DIR}/test/gdal_data
			COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/data      ${PROJECT_BINARY_DIR}/test/data)

else()
	add_custom_target(CopyResources ALL
			COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/proj_data  ${PROJECT_BINARY_DIR}/src/proj_data
			COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/gdal_data ${PROJECT_BINARY_DIR}/src/gdal_data)
endif()