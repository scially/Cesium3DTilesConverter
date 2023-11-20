file(GLOB Cesium_Sources
	"${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)

file(GLOB Cesium_Headers
	"${PROJECT_SOURCE_DIR}/include/CesiumMath/*.h"
)

add_library(CesiumMath
	STATIC 
	${Cesium_Sources}
	${Cesium_Headers}
)

target_compile_features(
	CesiumMath PUBLIC cxx_std_17
)

target_include_directories(
	CesiumMath
	PUBLIC 
	"${PROJECT_SOURCE_DIR}/include"
	${OPENSCENEGRAPH_INCLUDE_DIRS}
	${GDAL_INCLUDE_DIRS}
)

target_link_libraries(CesiumMath
	PUBLIC 
	Qt5::Core 
	${GDAL_LIBRARIES}
)