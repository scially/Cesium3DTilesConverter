file(GLOB Cesium3DTiles_Sources
	"${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)

file(GLOB Cesium3DTiles_Headers
	"${PROJECT_SOURCE_DIR}/include/Cesium3DTiles/*.h"
)
add_library(Cesium3DTiles 
	STATIC 
	${Cesium3DTiles_Headers}
	${Cesium3DTiles_Sources}
)

target_compile_features(
	Cesium3DTiles PUBLIC cxx_std_17
)

target_include_directories(
	Cesium3DTiles PUBLIC 
	"${PROJECT_SOURCE_DIR}/include"
)

target_link_libraries(Cesium3DTiles 
	PUBLIC
	Qt5::Core
)