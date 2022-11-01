cmake_minimum_required (VERSION 3.8)
project(Cesium3DTilesConverter)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_DEBUG_POSTFIX "d")
set(CMAKE_RELEASE_POSTFIX "")

if(MSVC)
	add_definitions(-DNOMINMAX)
	set(EXECUTABLE_OUTPUT_PATH  "${PROJECT_SOURCE_DIR}/binary/windows")
	set(LIBRARY_OUTPUT_PATH     "${PROJECT_SOURCE_DIR}/binary/windows")
else()
	set(EXECUTABLE_OUTPUT_PATH  "${PROJECT_SOURCE_DIR}/binary/linux")
	set(LIBRARY_OUTPUT_PATH     "${PROJECT_SOURCE_DIR}/binary/linux")
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
	set(EXECUTABLE_OUTPUT_PATH  "${EXECUTABLE_OUTPUT_PATH}/debug")
	set(LIBRARY_OUTPUT_PATH     "${LIBRARY_OUTPUT_PATH}/debug")
else()
	set(EXECUTABLE_OUTPUT_PATH  "${EXECUTABLE_OUTPUT_PATH}/release")
	set(LIBRARY_OUTPUT_PATH     "${LIBRARY_OUTPUT_PATH}/release")
endif()

find_package(GDAL REQUIRED)
find_package(Qt5 
	REQUIRED
	COMPONENTS Core Xml Test)
find_package(OpenSceneGraph 
	REQUIRED 
	COMPONENTS osgDB osgUtil)

include_directories(
	include
	${OPENSCENEGRAPH_INCLUDE_DIRS} 
	${GDAL_INCLUDE_DIR})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_library(Core STATIC
		"include/CoordinateConvert.h"
		"include/OGRException.h"
		"include/OSGBPageLodVisitor.h"
		"include/OSGBLevel.h"
		"include/BoundingVolume.h"
		"include/BoundingVolumeBox.h"
		"include/BoundingVolumeRegion.h"
		"include/AssetProperties.h"
		"include/BoundingVolumeSphere.h"
		"include/TileMatrix.h"
		"include/Refine.h"
		"include/ContentTile.h"
		"include/RootTile.h"
		"include/BaseTile.h"
		"include/OSGBuildState.h"
		"include/stb_image.h"
		"include/stb_image_write.h"
		"include/tiny_gltf.h"
		"include/TilesParseException.h"
		"include/TilesConvertException.h"
		"include/OSGBConvertJob.h"
		"include/ShpConvertJob.h"
		"include/ModelMetadata.h"
		"include/QuadTree.h"
		"include/ShpConvert.h"
		"include/GeometryMesh.h"
		"include/GDALWrapper.h"
		"include/Batched3DModel.h"
		"include/Utils.h"
		"include/earcut.hpp"

		"src/CoordinateConvert.cpp"
		"src/OSGBPageLodVisitor.cpp"
		"src/BoundingVolume.cpp"
		"src/BoundingVolumeBox.cpp"
		"src/BoundingVolumeRegion.cpp"
		"src/BoundingVolumeSphere.cpp"
		"src/AssetProperties.cpp"
		"src/TileMatrix.cpp"
		"src/Refine.cpp"
		"src/ContentTile.cpp"
		"src/RootTile.cpp"
		"src/BaseTile.cpp"
		"src/DxtImage.cpp"
		"src/OSGBuildState.cpp"
		"src/OSGBLevel.cpp"
		"src/OSGBConvert.cpp"
		"src/OSGBConvertJob.cpp"
		"src/ModelMetadata.cpp"
		"src/QuadTree.cpp"
		"src/GeometryMesh.cpp"
		"src/Batched3DModel.cpp"
		"src/ShpConvert.cpp"
		"src/ShpConvertJob.cpp")

target_link_libraries(Core
		${GDAL_LIBRARIES}
		${PROJ_LIBRARIES}
		${OPENSCENEGRAPH_LIBRARIES}
		Qt5::Xml
		Qt5::Core)

if(UNIX)
	target_link_libraries(Core -lGL)
endif()

add_executable(Converter src/main.cpp)
target_link_libraries(Converter
		Core)

add_custom_command(TARGET Converter
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/proj_data  ${EXECUTABLE_OUTPUT_PATH}/proj_data
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/gdal_data  ${EXECUTABLE_OUTPUT_PATH}/gdal_data
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Qt5::Xml>  $<TARGET_FILE_DIR:Converter>
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Qt5::Core>  $<TARGET_FILE_DIR:Converter>
)