
add_library(3dtiles_core STATIC 
	"../include/3dtiles_core.h" "3dtiles_core.cpp" 
	"../include/gdal_tools.h" "gdal_tools.cpp"
	"../include/osg_converter.h" "osg_converter.cpp" )


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