file(GLOB CesiumReadWrite_Sources
	"${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)

file(GLOB CesiumReadWrite_Headers
	"${PROJECT_SOURCE_DIR}/include/CesiumReadWrite/*.h"
)
add_library(CesiumReadWrite 
	STATIC 
	${CesiumReadWrite_Sources}
	${CesiumReadWrite_Headers}
)

target_compile_features(
	CesiumReadWrite PUBLIC cxx_std_17
)

target_include_directories(
	CesiumReadWrite PUBLIC 
	"${PROJECT_SOURCE_DIR}/include"
	${OPENSCENEGRAPH_INCLUDE_DIRS}
)

target_link_libraries(CesiumReadWrite 
	PUBLIC 
	Cesium3DTiles
	${OPENSCENEGRAPH_LIBRARIES}
)