include(QtRuntimeLibraryFunction)

file(GLOB ConvertApp_Sources
	"${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)

file(GLOB ConvertApp_Headers
	"${PROJECT_SOURCE_DIR}/include/ConvertApp/*.h"
)

add_executable(ConvertApp 
	${ConvertApp_Sources}
	${ConvertApp_Headers}
)

target_compile_features(
	ConvertApp PUBLIC cxx_std_17
)

target_include_directories(
	ConvertApp PUBLIC 
	"${PROJECT_SOURCE_DIR}/include/ConvertApp"
)

target_link_libraries(ConvertApp 
	PUBLIC 
	OSGConvert
)

link_qt_library(ConvertApp)
copy_gdal_data(ConvertApp)