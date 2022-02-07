# Install script for directory: D:/vcpkg/buildtrees/proj4/src/8.1.1-e5312ceaed.clean/data

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "D:/vcpkg/packages/proj4_x86-windows/debug")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/proj4" TYPE FILE FILES
    "D:/vcpkg/buildtrees/proj4/src/8.1.1-e5312ceaed.clean/data/proj.ini"
    "D:/vcpkg/buildtrees/proj4/src/8.1.1-e5312ceaed.clean/data/world"
    "D:/vcpkg/buildtrees/proj4/src/8.1.1-e5312ceaed.clean/data/other.extra"
    "D:/vcpkg/buildtrees/proj4/src/8.1.1-e5312ceaed.clean/data/nad27"
    "D:/vcpkg/buildtrees/proj4/src/8.1.1-e5312ceaed.clean/data/GL27"
    "D:/vcpkg/buildtrees/proj4/src/8.1.1-e5312ceaed.clean/data/nad83"
    "D:/vcpkg/buildtrees/proj4/src/8.1.1-e5312ceaed.clean/data/nad.lst"
    "D:/vcpkg/buildtrees/proj4/src/8.1.1-e5312ceaed.clean/data/CH"
    "D:/vcpkg/buildtrees/proj4/src/8.1.1-e5312ceaed.clean/data/ITRF2000"
    "D:/vcpkg/buildtrees/proj4/src/8.1.1-e5312ceaed.clean/data/ITRF2008"
    "D:/vcpkg/buildtrees/proj4/src/8.1.1-e5312ceaed.clean/data/ITRF2014"
    "D:/vcpkg/buildtrees/proj4/x86-windows-dbg/data/proj.db"
    "D:/vcpkg/buildtrees/proj4/src/8.1.1-e5312ceaed.clean/data/deformation_model.schema.json"
    "D:/vcpkg/buildtrees/proj4/src/8.1.1-e5312ceaed.clean/data/projjson.schema.json"
    "D:/vcpkg/buildtrees/proj4/src/8.1.1-e5312ceaed.clean/data/triangulation.schema.json"
    )
endif()

