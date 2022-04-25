
FILE(GLOB TestUnits "Test*.cpp")

foreach(TestUnit IN LISTS TestUnits)
    get_filename_component(TestUnitName ${TestUnit} NAME_WE)
    add_executable(${TestUnitName} ${TestUnit})
    target_link_libraries(${TestUnitName}
            Qt5::Test Core)
endforeach()