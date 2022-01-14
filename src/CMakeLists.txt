
add_library(3dtiles_core STATIC 
	"../include/CoordinateConvert.h"
	"../include/OGRException.h"
	"../include/OSGBPageLodVisitor.h"
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
	"OSGBuildState.cpp" "OSGBLevel.cpp")


target_link_libraries(3dtiles_core
	${GDAL_LIBRARY}
	${OPENSCENEGRAPH_LIBRARIES}
	Qt5::Core 
	Qt5::Xml
	Qt5::Gui)


#add_executable(3dtiles_cli "main.cpp")

#target_link_libraries(3dtiles_cli 3dtiles_core)

#add_custom_command(
#        TARGET 3dtiles_cli POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_directory
#                ${PROJECT_SOURCE_DIR}/gdal_data
#                ${PROJECT_BINARY_DIR}/gdal_data)