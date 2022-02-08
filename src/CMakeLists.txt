
add_library(Core STATIC
	"../include/CoordinateConvert.h"
	"../include/OGRException.h"
	"../include/OSGBPageLodVisitor.h"
	"../include/OSGBLevel.h"
	"../include/BoundingVolume.h"
	"../include/BoundingVolumeBox.h"
	"../include/BoundingVolumeRegion.h"
	"../include/AssetProperties.h"
	"../include/BoundingVolumeSphere.h"
	"../include/TileMatrix.h"
	"../include/Refine.h"
	"../include/ContentTile.h"
	"../include/RootTile.h"
	"../include/BaseTile.h"
	"../include/OSGBuildState.h"
	"../include/stb_image.h"
	"../include/stb_image_write.h"
	"../include/tiny_gltf.h"
	"../include/TilesParseException.h"
	"../include/TilesConvertException.h"
    "../include/OSGBConvertJob.h"
	"../include/ShpConvertJob.h"
    "../include/ModelMetadata.h"
    "../include/QuadTree.h"
    "../include/ShpConvert.h"
    "../include/GeometryMesh.h"
    "../include/GDALWrapper.h"
	"../include/Batched3DModel.h"
	"../include/Utils.h"
    "../include/earcut.hpp"

	"CoordinateConvert.cpp"
	"OSGBPageLodVisitor.cpp" 
	"BoundingVolume.cpp" 
	"BoundingVolumeBox.cpp" 
	"BoundingVolumeRegion.cpp" 
	"BoundingVolumeSphere.cpp" 
	"AssetProperties.cpp" 
	"TileMatrix.cpp" 
	"Refine.cpp" 
	"ContentTile.cpp" 
	"RootTile.cpp"
	"BaseTile.cpp" 
	"DxtImage.cpp"
	"OSGBuildState.cpp" 
	"OSGBLevel.cpp" 
	"OSGBConvert.cpp" 
    "OSGBConvertJob.cpp"
    "ModelMetadata.cpp"
    "QuadTree.cpp"
    "GeometryMesh.cpp"
    "GDALWrapper.cpp"
	"Batched3DModel.cpp"
	"ShpConvert.cpp" 
	"ShpConvertJob.cpp")


target_link_libraries(Core
	${GDAL_LIBRARY}
	${OPENSCENEGRAPH_LIBRARIES}
	Qt5::Core 
	Qt5::Xml)


add_executable(Converter "main.cpp")
target_link_libraries(Converter
    ${GDAL_LIBRARY}
    ${OPENSCENEGRAPH_LIBRARIES}
    Qt5::Core
    Qt5::Xml
    Core)

add_custom_command(
        TARGET Converter PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${PROJECT_SOURCE_DIR}/proj_data/proj.db
                ${CMAKE_BINARY_DIR}/src/proj.db
        COMMAND ${CMAKE_COMMAND} -E copy
                ${PROJECT_SOURCE_DIR}/proj_data/proj.ini
                ${CMAKE_BINARY_DIR}/src/proj.ini)

