macro(link_qt_library target)
    if (WIN32)
        get_target_property(QT5_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
        get_filename_component(QT5_WINDEPLOYQT_EXECUTABLE ${QT5_QMAKE_EXECUTABLE} PATH)
        set(QT5_WINDEPLOYQT_EXECUTABLE "${QT5_WINDEPLOYQT_EXECUTABLE}/windeployqt.exe")

        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${QT5_WINDEPLOYQT_EXECUTABLE} $<TARGET_FILE_DIR:${target}>)
    endif(WIN32)
endmacro(link_qt_library)

macro(copy_gdal_data target)
    add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
                    "${PROJECT_SOURCE_DIR}/share/proj_data"
                    $<TARGET_FILE_DIR:${target}>/proj_data
                COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
                    "${PROJECT_SOURCE_DIR}/share/gdal_data"
                    $<TARGET_FILE_DIR:${target}>/gdal_data
    )
endmacro(copy_gdal_data)