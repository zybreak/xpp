cmake_minimum_required(VERSION 3.0)
project(xpp)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

#
# Process dependencies
#
find_package(PkgConfig)
pkg_check_modules(XCBPROTO REQUIRED xcb-proto)

find_package(PythonInterp 2.7 REQUIRED)
find_package(X11 REQUIRED)
find_package(X11_XCB REQUIRED)
find_package(XCB REQUIRED)

set(XPP_INCLUDE_DIRS
  ${X11_INCLUDE_DIRS}
  ${X11_XCB_INCLUDE_DIR}
  ${XCB_XCB_INCLUDE_DIR})
set(XPP_LIBRARIES
  ${X11_X11_LIB}
  ${X11_XCB_LIBRARIES}
  ${XCB_XCB_LIBRARY})

#
# Include XCB libs depending on what protos we build
#
if(NOT XCB_PROTOS)
  set(XCB_PROTOS
    "bigreq"
    "composite"
    "damage"
    "dpms"
    "dri2"
    "dri3"
    "glx"
    "present"
    "randr"
    "record"
    "render"
    "res"
    "screensaver"
    "shape"
    "shm"
    "sync"
    "xc_misc"
    "xevie"
    "xf86dri"
    "xfixes"
    "xinerama"
    "xinput"
    "xkb"
    "xprint"
    "xproto"
    "xselinux"
    "xtest"
    "xv"
    "xvmc")
endif()
if(";${XCB_PROTOS};" MATCHES ";randr;")
  set(XPP_INCLUDE_DIRS ${XPP_INCLUDE_DIRS} ${XCB_RANDR_INCLUDE_DIR})
  set(XPP_LIBRARIES ${XPP_LIBRARIES} ${XCB_RANDR_LIBRARY})
endif()

set(PROTO_LIST)

# FIXME: Find replacement for the hardcoded paths
file(GLOB PROTO_LIST_RAW RELATIVE /usr/share/xcb /usr/share/xcb/*.xml)

#
# Filter glob
#
foreach(PROTO_RAW ${PROTO_LIST_RAW})
  string(REGEX REPLACE "(^xf86vidmode.xml|^ge.xml|.xml)\$" "" PROTO ${PROTO_RAW})
  if(PROTO AND ";${XCB_PROTOS};" MATCHES ";${PROTO};")
    message(STATUS "${PROJECT_NAME}: including xcb proto ${PROTO_RAW}")
    set(PROTO_LIST ${PROTO_LIST} ${PROTO})
  endif()
endforeach(PROTO_RAW)

#
# Create project lib and commands
#
file(GLOB HEADER_FILES include/*.hpp)
add_library(${PROJECT_NAME} ${HEADER_FILES})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

target_include_directories(${PROJECT_NAME} PUBLIC
  ${PROJECT_SOURCE_DIR}/include
  ${X11_INCLUDE_DIRS}
  ${X11_XCB_INCLUDE_DIR}
  ${XCB_XCB_INCLUDE_DIR})

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wpedantic)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:-g3 -DDEBUG>)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:-O3 -Wno-unused-variable>)

target_compile_definitions(${PROJECT_NAME} PUBLIC ${X11_XCB_DEFINITIONS} ${XCB_DEFINITIONS})

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(${PROJECT_NAME} ${XPP_LIBRARIES})

#
# Add commands
#
foreach(PROTO ${PROTO_LIST})
  string(REGEX REPLACE "proto\$" "" PROTO_OUTPUT ${PROTO})
  add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
    # FIXME: Find replacement for the hardcoded paths
    COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/generators/cpp_client.py
      -p /usr/lib/python2.7/site-packages
      -p /usr/lib/python2.7/dist-packages
      /usr/share/xcb/${PROTO}.xml > ${PROJECT_SOURCE_DIR}/include/proto/${PROTO_OUTPUT}.hpp)
endforeach(PROTO)

#
# Export lists to the parent scope if there are any
#
get_directory_property(HAS_PARENT PARENT_DIRECTORY)

if(HAS_PARENT)
  set(XPP_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include PARENT_SCOPE)
  set(XPP_LIBRARIES ${PROJECT_NAME} PARENT_SCOPE)
endif()
