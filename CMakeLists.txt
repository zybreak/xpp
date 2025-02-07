cmake_minimum_required(VERSION 3.29.20240416 FATAL_ERROR)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")

project(xpp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_MODULE_STD ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

#
# Process dependencies
#
find_package(PkgConfig)
pkg_check_modules(XCBPROTO REQUIRED xcb-proto)

# Store the location of the proto xml files inside XCBPROTO_XCBINCLUDEDIR
pkg_get_variable(XCBPROTO_XCBINCLUDEDIR xcb-proto xcbincludedir)

# Search for a python interpreter, if the user didn't specify any
if(NOT PYTHON_EXECUTABLE)
  find_package(Python3 REQUIRED COMPONENTS Interpreter)
  if(Python3_FOUND)
    set(PYTHON_EXECUTABLE "${Python3_EXECUTABLE}")
  endif()
endif()

if(NOT PYTHON_EXECUTABLE)
  message(FATAL_ERROR "Missing PYTHON_EXECUTABLE")
endif()

find_package(XCB REQUIRED XCB ICCCM EWMH UTIL IMAGE)

set(GEN_SRC "${CMAKE_CURRENT_BINARY_DIR}/generated-sources")

set(XPP_INCLUDE_DIRS
  ${XCB_XCB_INCLUDE_DIR}
  ${XCB_EWMH_INCLUDE_DIR}
  ${XCB_ICCCM_INCLUDE_DIR}
  ${XCB_UTIL_INCLUDE_DIR}
  ${XCB_IMAGE_INCLUDE_DIR})
set(XPP_LIBRARIES
  ${XCB_XCB_LIBRARY}
  ${XCB_EWMH_LIBRARY}
  ${XCB_ICCCM_LIBRARY}
  ${XCB_UTIL_LIBRARY}
  ${XCB_IMAGE_LIBRARY})

#
# Loop through a hardcoded list of python executables to locate the python module "xcbgen"
#
# TODO drop python2 once ubuntu and debian ship python3-xcbgen in their
# maintained distros
foreach(CURRENT_EXECUTABLE ${PYTHON_EXECUTABLE} python3 python python2 python2.7)
  message(STATUS "Searching for xcbgen with " ${CURRENT_EXECUTABLE})

  execute_process(COMMAND "${CURRENT_EXECUTABLE}" "-c"
    "import re,xcbgen;print(re.compile('/xcbgen/__init__.py.*').sub('',xcbgen.__file__))"
    RESULT_VARIABLE _xcbgen_status
    OUTPUT_VARIABLE _xcbgen_location
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

  # When a shell script returns successfully its return code is 0
  if(_xcbgen_status EQUAL 0)
    set(PYTHON_XCBGEN "${_xcbgen_location}" CACHE STRING "Location of python module: xcbgen ")
    message(STATUS "Found xcbgen in " ${PYTHON_XCBGEN})
    break()
  endif()

endforeach(CURRENT_EXECUTABLE)

if(NOT PYTHON_XCBGEN)
  message(FATAL_ERROR "Missing required python module: xcbgen")
endif()

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

if("randr" IN_LIST XCB_PROTOS)
  find_package(XCB REQUIRED RANDR)
  set(XPP_INCLUDE_DIRS ${XPP_INCLUDE_DIRS} ${XCB_RANDR_INCLUDE_DIR})
  set(XPP_LIBRARIES ${XPP_LIBRARIES} ${XCB_RANDR_LIBRARY})
endif()
if("render" IN_LIST XCB_PROTOS)
  find_package(XCB REQUIRED RENDER)
  set(XPP_INCLUDE_DIRS ${XPP_INCLUDE_DIRS} ${XCB_RENDER_INCLUDE_DIR})
  set(XPP_LIBRARIES ${XPP_LIBRARIES} ${XCB_RENDER_LIBRARY})
endif()
if("damage" IN_LIST XCB_PROTOS)
  find_package(XCB REQUIRED DAMAGE)
  set(XPP_INCLUDE_DIRS ${XPP_INCLUDE_DIRS} ${XCB_DAMAGE_INCLUDE_DIR})
  set(XPP_LIBRARIES ${XPP_LIBRARIES} ${XCB_DAMAGE_LIBRARY})
endif()
if("sync" IN_LIST XCB_PROTOS)
  find_package(XCB REQUIRED SYNC)
  set(XPP_INCLUDE_DIRS ${XPP_INCLUDE_DIRS} ${XCB_SYNC_INCLUDE_DIR})
  set(XPP_LIBRARIES ${XPP_LIBRARIES} ${XCB_SYNC_LIBRARY})
endif()
if("composite" IN_LIST XCB_PROTOS)
  find_package(XCB REQUIRED COMPOSITE)
  set(XPP_INCLUDE_DIRS ${XPP_INCLUDE_DIRS} ${XCB_COMPOSITE_INCLUDE_DIR})
  set(XPP_LIBRARIES ${XPP_LIBRARIES} ${XCB_COMPOSITE_LIBRARY})
endif()
if("xkb" IN_LIST XCB_PROTOS)
  find_package(XCB REQUIRED XKB)
  set(XPP_INCLUDE_DIRS ${XPP_INCLUDE_DIRS} ${XCB_XKB_INCLUDE_DIR})
  set(XPP_LIBRARIES ${XPP_LIBRARIES} ${XCB_XKB_LIBRARY})
endif()

set(PROTO_LIST)

file(GLOB PROTO_LIST_RAW RELATIVE ${XCBPROTO_XCBINCLUDEDIR} ${XCBPROTO_XCBINCLUDEDIR}/*.xml)

#
# Filter glob
#
foreach(PROTO_RAW ${PROTO_LIST_RAW})
  string(REGEX REPLACE "(^xf86vidmode.xml|^ge.xml|.xml)\$" "" PROTO ${PROTO_RAW})
  if(PROTO AND "${PROTO}" IN_LIST XCB_PROTOS)
    message(STATUS "${PROJECT_NAME}: including xcb proto ${PROTO_RAW}")
    set(PROTO_LIST ${PROTO_LIST} ${PROTO})
  endif()
endforeach(PROTO_RAW)

#
# Add commands
#
set(PROTO_GEN_DIR "${GEN_SRC}/xpp/proto")
file(MAKE_DIRECTORY ${PROTO_GEN_DIR})
set(PROTO_GEN_FILES "")
foreach(PROTO ${PROTO_LIST})
    string(REGEX REPLACE "proto\$" "" PROTO_OUTPUT ${PROTO})
    set(OUTPUT_FILE ${PROTO_GEN_DIR}/${PROTO_OUTPUT}.cppm)
    #execute_process(
    #        OUTPUT_FILE ${OUTPUT_FILE}
    #        COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/generators/cpp_client.py -p "${PYTHON_XCBGEN}"
    #        ${XCBPROTO_XCBINCLUDEDIR}/${PROTO}.xml > ${OUTPUT_FILE}
    #        RESULT_VARIABLE result)
    #if(NOT result EQUAL 0)
    #    message(FATAL_ERROR "xcb-gen failed")
    #endif()
    add_custom_command(
            OUTPUT ${OUTPUT_FILE}
            COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/generators/cpp_client.py -p "${PYTHON_XCBGEN}"
            ${XCBPROTO_XCBINCLUDEDIR}/${PROTO}.xml > ${OUTPUT_FILE})
    list(APPEND PROTO_GEN_FILES ${OUTPUT_FILE})
endforeach(PROTO)


#
# Create project lib and commands
#
add_library(${PROJECT_NAME} STATIC)
target_sources(${PROJECT_NAME} PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES
        BASE_DIRS src ${GEN_SRC}
        FILES
        src/xpp.cppm
        src/generic.cppm
        src/generic/error.cppm
        src/generic/event.cppm
        src/generic/factory.cppm
        src/generic/signature.cppm
        src/generic/request.cppm
        src/generic/resource.cppm
        src/generic/extension.cppm
        src/generic/iterator_traits.cppm
        src/generic/reply_iterator.cppm
        src/generic/input_iterator_adapter.cppm
        src/core.cppm
        src/atom.cppm
        src/event.cppm
        src/colormap.cppm
        src/cursor.cppm
        src/drawable.cppm
        src/font.cppm
        src/fontable.cppm
        src/gcontext.cppm
        src/pixmap.cppm
        src/window.cppm
        src/connection.cppm
        ${PROTO_GEN_FILES}
)
target_include_directories(${PROJECT_NAME} INTERFACE ${XPP_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} INTERFACE ${XPP_LIBRARIES})
target_compile_options(${PROJECT_NAME} INTERFACE ${XCB_DEFINITIONS})
