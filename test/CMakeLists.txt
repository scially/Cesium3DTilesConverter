
# TestGDAL
add_executable(TestGDAL "TestGDAL.cpp")
target_link_libraries(TestGDAL Qt5::Test 3dtiles_core)
add_test(name TestGDAL command TestGDAL)

# Test3DTiles
add_executable(Test3DTiles "Test3DTiles.cpp")
target_link_libraries(Test3DTiles Qt5::Test 3dtiles_core)
add_test(name Test3DTiles command Test3DTiles)

# TestOSGBLevel
add_executable(TestOSGBConvert "TestOSGBConvert.cpp")
target_link_libraries(TestOSGBConvert Qt5::Test 3dtiles_core)
add_test(name TestOSGBConvert command TestOSGBConvert)

add_custom_command(
        TARGET TestOSGBConvert 
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/data
                ${CMAKE_BINARY_DIR}/test/data)
