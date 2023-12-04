find_package(Qt5 REQUIRED COMPONENTS Test)
include(QtRuntimeLibraryFunction)

set(TESTS 
	TestOSGLodVisitor.cpp
	TestOSGMergTopIndex.cpp
	TestOSGParseVisitor.cpp)

foreach(TESTSRC ${TESTS})
	get_filename_component(TESTNAME ${TESTSRC} NAME_WE)
	add_executable(${TESTNAME} ${TESTSRC} "TestTemplate.h")
	add_test(NAME ${TESTNAME} COMMAND ${TESTNAME})
	target_link_libraries(${TESTNAME} 
		PRIVATE 
		Qt5::Test 
		OSGConvert)
	link_qt_library(${TESTNAME})
	copy_gdal_data(${TESTNAME})
	
	# copy test data
	add_custom_command(TARGET  ${TESTNAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
                "${PROJECT_SOURCE_DIR}/data"
                $<TARGET_FILE_DIR:${TESTNAME}>/data
    )
endforeach(TESTSRC)

