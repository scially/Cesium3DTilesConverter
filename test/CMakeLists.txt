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