
# test gdal_tool
add_executable(TestGDAL "TestGDAL.cpp")
target_link_libraries(TestGDAL Qt5::Test 3dtiles_core)
add_test(name TestGDAL command TestGDAL)

# test gdal_3dtiles
add_executable(Test3DTiles "Test3DTiles.cpp")
target_link_libraries(Test3DTiles Qt5::Test 3dtiles_core)
add_test(name Test3DTiles command Test3DTiles)


# test gdal_3dtiles
add_executable(TestOSGBLevel "TestOSGBLevel.cpp")
target_link_libraries(TestOSGBLevel Qt5::Test 3dtiles_core)
add_test(name TestOSGBLevel command TestOSGBLevel)