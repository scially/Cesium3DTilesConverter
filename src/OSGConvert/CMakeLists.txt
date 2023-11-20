file(GLOB OSGConvert_Sources
	"${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)

file(GLOB OSGConvert_Headers
	"${PROJECT_SOURCE_DIR}/include/OSGBConvert/*.h"
)

add_library(OSGConvert 
	STATIC 
	${OSGConvert_Sources}
	${OSGConvert_Headers}
)

target_compile_features(
	OSGConvert PUBLIC cxx_std_17
)

target_include_directories(
	OSGConvert PUBLIC 
	"${PROJECT_SOURCE_DIR}/include/OSGConvert"
	${OPENSCENEGRAPH_INCLUDE_DIRS}
)

target_link_libraries(OSGConvert 
	PUBLIC 
	Qt5::Core 
	Qt5::Xml
	Qt5::Concurrent
	CesiumReadWrite
	CesiumMath
	CesiumGLTF
    Commons
	${OPENSCENEGRAPH_LIBRARIES}
)
