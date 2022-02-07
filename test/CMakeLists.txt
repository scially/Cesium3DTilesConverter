
# TestGDAL
add_executable(TestGDAL "TestGDAL.cpp")
target_link_libraries(TestGDAL Qt5::Test Core)
add_test(name TestGDAL command TestGDAL)

# Test3DTiles
add_executable(Test3DTiles "Test3DTiles.cpp")
target_link_libraries(Test3DTiles Qt5::Test Core)
add_test(name Test3DTiles command Test3DTiles)

# TestOSGBConvert
add_executable(TestOSGBConvert "TestOSGBConvert.cpp")
target_link_libraries(TestOSGBConvert Qt5::Test Core)
add_test(name TestOSGBConvert command TestOSGBConvert)

# TestOSGBLevel
add_executable(TestOSGBLevel "TestOSGBLevel.cpp")
target_link_libraries(TestOSGBLevel Qt5::Test Core)
add_test(name TestOSGBLevel command TestOSGBLevel)

# TestOSGBLevel
add_executable(TestOSGBConvertJob "TestOSGBConvertJob.cpp")
target_link_libraries(TestOSGBConvertJob Qt5::Test Core)
add_test(name TestOSGBConvertJob command TestOSGBConvertJob)

#add_custom_command(
#        TARGET CopyTestResources PRE_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_directory
#                ${CMAKE_CURRENT_SOURCE_DIR}/data
#                ${CMAKE_BINARY_DIR}/test/data)

add_custom_target(CopyTestResources ALL)
add_custom_command(
        TARGET CopyTestResources PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${PROJECT_SOURCE_DIR}/proj_data/proj.db
                ${CMAKE_BINARY_DIR}/test/proj.db
        COMMAND ${CMAKE_COMMAND} -E copy
                ${PROJECT_SOURCE_DIR}/proj_data/proj.ini
                ${CMAKE_BINARY_DIR}/test/proj.ini)

