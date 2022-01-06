
# test gdal_tool
add_executable(test_gdal test_gdal.cpp "test_gdal.cpp")
target_link_libraries(test_gdal Qt5::Test 3dtiles_core)
add_test(NAME test_gdal COMMAND test_gdal)

# test gdal_3dtiles
add_executable(test_3dtiles test_3dtiles.cpp "test_3dtiles.cpp")
target_link_libraries(test_3dtiles Qt5::Test 3dtiles_core)
add_test(NAME test_3dtiles COMMAND test_3dtiles)