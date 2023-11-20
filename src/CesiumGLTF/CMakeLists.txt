file(GLOB Cesium_Sources
	"${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)

file(GLOB Cesium_Headers
	"${PROJECT_SOURCE_DIR}/include/CesiumGLTF/*.h"
)

add_library(CesiumGLTF
	STATIC 
	${Cesium_Sources}
	${Cesium_Headers}
)

target_compile_features(
	CesiumGLTF PUBLIC cxx_std_17
)

target_include_directories(
	CesiumGLTF
	PUBLIC 
	"${PROJECT_SOURCE_DIR}/include"
	${OPENSCENEGRAPH_INCLUDE_DIRS}
)

target_link_libraries(CesiumGLTF
	PUBLIC 
	Qt5::Core 
	tinygltf
)