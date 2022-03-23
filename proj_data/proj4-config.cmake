# Configure PROJ
#
# Set
#  PROJ4_FOUND = 1
#  PROJ4_INCLUDE_DIRS = /usr/local/include
#  PROJ4_LIBRARIES = PROJ4::proj
#  PROJ4_LIBRARY_DIRS = /usr/local/lib
#  PROJ4_BINARY_DIRS = /usr/local/bin
#  PROJ4_VERSION = 4.9.1 (for example)
cmake_policy(PUSH)
cmake_policy(SET CMP0012 NEW)
include(CMakeFindDependencyMacro)
find_dependency(unofficial-sqlite3 CONFIG)
if("ON")
    find_dependency(TIFF)
endif()
if("ON")
    find_dependency(CURL CONFIG)
endif()
cmake_policy(POP)

# Tell the user project where to find our headers and libraries
get_filename_component (_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)
get_filename_component (_ROOT "${_DIR}/../../../" ABSOLUTE)
set (PROJ4_INCLUDE_DIRS "${_ROOT}/include")
set (PROJ4_LIBRARY_DIRS "${_ROOT}/lib")
set (PROJ4_BINARY_DIRS "${_ROOT}/bin")

set (PROJ4_LIBRARIES PROJ4::proj)
# Read in the exported definition of the library
include ("${_DIR}/proj-targets.cmake")
include ("${_DIR}/proj4-targets.cmake")

unset (_ROOT)
unset (_DIR)

if ("PROJ4" STREQUAL "PROJ4")
  # For backward compatibility with old releases of libgeotiff
  set (PROJ4_INCLUDE_DIR
    ${PROJ4_INCLUDE_DIRS})
endif ()
