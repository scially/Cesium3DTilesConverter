file(GLOB Commons_Sources
	"${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)

file(GLOB Commons_Headers
	"${PROJECT_SOURCE_DIR}/include/CommonIO/*.h"
)

add_library(Commons
	STATIC 
    ${Commons_Sources}
    ${Commons_Headers}
)

target_compile_features(
    Commons PUBLIC cxx_std_17
)

target_include_directories(
    Commons PUBLIC
	"${PROJECT_SOURCE_DIR}/include"
	${GDAL_INCLUDE_DIRS}
	${OPENSCENEGRAPH_INCLUDE_DIRS}
)

target_link_libraries(Commons
	PUBLIC
	Qt5::Core
	${OPENSCENEGRAPH_LIBRARIES}
	${GDAL_LIBRARIES}
)
