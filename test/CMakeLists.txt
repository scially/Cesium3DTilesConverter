
# TestGDAL
add_executable(TestGDAL "TestGDAL.cpp")
target_link_libraries(TestGDAL Qt5::Test 3dtiles_core)
add_test(name TestGDAL command TestGDAL)

# Test3DTiles
add_executable(Test3DTiles "Test3DTiles.cpp")
target_link_libraries(Test3DTiles Qt5::Test 3dtiles_core)
add_test(name Test3DTiles command Test3DTiles)

# TestOSGBLevel
add_executable(TestOSGBLevel "TestOSGBLevel.cpp")
target_link_libraries(TestOSGBLevel Qt5::Test 3dtiles_core)
add_test(name TestOSGBLevel command TestOSGBLevel)

add_custom_command(
        TARGET TestOSGBLevel 
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/data
                ${CMAKE_BINARY_DIR}/test/data)
