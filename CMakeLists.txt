cmake_minimum_required (VERSION 3.8)
project(Cesium3DTilesConverter VERSION 2.1.0)

configure_file (
    "${PROJECT_SOURCE_DIR}/include/Config.h.in"
    "${PROJECT_SOURCE_DIR}/include/Config.h"
)

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

file(GLOB_RECURSE PROJECT_HEADERS
	"include/*.h"
	"include/*.hpp"
)

file(GLOB_RECURSE PROJECT_SRCS
	"src/*.cpp"
)

add_executable(Converter 
	${PROJECT_HEADERS} 
	${PROJECT_SRCS}
)

target_link_libraries(Converter
		${GDAL_LIBRARIES}
		${PROJ_LIBRARIES}
		${OPENSCENEGRAPH_LIBRARIES}
		Qt5::Xml
		Qt5::Core)

add_custom_command(TARGET Converter POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/share/proj_data  ${EXECUTABLE_OUTPUT_PATH}/proj_data
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/share/gdal_data  ${EXECUTABLE_OUTPUT_PATH}/gdal_data
)

if (WIN32)
    get_target_property(QT5_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
    get_filename_component(QT5_WINDEPLOYQT_EXECUTABLE ${QT5_QMAKE_EXECUTABLE} PATH)
    set(QT5_WINDEPLOYQT_EXECUTABLE "${QT5_WINDEPLOYQT_EXECUTABLE}/windeployqt.exe")

    add_custom_command(TARGET Converter POST_BUILD
       COMMAND ${QT5_WINDEPLOYQT_EXECUTABLE} $<TARGET_FILE_DIR:Converter>)
endif(WIN32)