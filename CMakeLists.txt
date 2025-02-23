cmake_minimum_required(VERSION 3.31 FATAL_ERROR)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")

# Determine if fmt is built as a subproject (using add_subdirectory)
# or if it is the master project.
if (NOT DEFINED XPP_MASTER_PROJECT)
  set(XPP_MASTER_PROJECT OFF)
  if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(XPP_MASTER_PROJECT ON)
    message(STATUS "CMake version: ${CMAKE_VERSION}")
  endif ()
endif ()

if (XPP_MASTER_PROJECT AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Release type")
endif ()

if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 23)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif ()

set(CMAKE_CXX_MODULE_STD ON)

project(xpp VERSION 1.0.0 LANGUAGES CXX)

if (XPP_MASTER_PROJECT AND NOT DEFINED CMAKE_CXX_VISIBILITY_PRESET)
  #set(CMAKE_CXX_VISIBILITY_PRESET hidden)
endif ()

if (XPP_MASTER_PROJECT AND NOT DEFINED CMAKE_VISIBILITY_INLINES_HIDDEN)
  #set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
endif ()

#
#string(APPEND CMAKE_CXX_FLAGS " -fexperimental-modules-reduced-bmi -Xclang -fno-validate-pch")
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

find_package(X11 REQUIRED COMPONENTS xcb xcb_icccm xcb_ewmh xcb_util xcb_image)
list(APPEND X11_DEPENDENCIES xcb xcb_icccm xcb_ewmh xcb_util xcb_image)
list(APPEND libraries X11 X11::xcb X11::xcb_icccm X11::xcb_ewmh X11::xcb_util X11::xcb_image)

set(GEN_SRC "${CMAKE_CURRENT_BINARY_DIR}/generated-sources")

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
  find_package(X11 REQUIRED COMPONENTS xcb_randr)
  list(APPEND X11_DEPENDENCIES xcb_randr)
  list(APPEND libraries X11::xcb_randr)
endif()
if("render" IN_LIST XCB_PROTOS)
  find_package(X11 REQUIRED COMPONENTS xcb_render)
  list(APPEND X11_DEPENDENCIES xcb_render)
  list(APPEND libraries X11::xcb_render)
endif()
if("damage" IN_LIST XCB_PROTOS)
  find_package(X11 REQUIRED COMPONENTS xcb_damage)
  list(APPEND X11_DEPENDENCIES xcb_damage)
  list(APPEND libraries X11::xcb_damage)
endif()
if("sync" IN_LIST XCB_PROTOS)
  find_package(X11 REQUIRED COMPONENTS xcb_sync)
  list(APPEND X11_DEPENDENCIES xcb_sync)
  list(APPEND libraries X11::xcb_sync)
endif()
if("composite" IN_LIST XCB_PROTOS)
  find_package(X11 REQUIRED COMPONENTS xcb_composite)
  list(APPEND X11_DEPENDENCIES xcb_composite)
  list(APPEND libraries X11::xcb_composite)
endif()
if("xkb" IN_LIST XCB_PROTOS)
  find_package(X11 REQUIRED COMPONENTS xcb_xkb)
  list(APPEND X11_DEPENDENCIES xcb_xkb)
  list(APPEND libraries X11::xcb_xkb)
endif()

set(PROTO_LIST)

file(GLOB PROTO_LIST_RAW RELATIVE ${XCBPROTO_XCBINCLUDEDIR} ${XCBPROTO_XCBINCLUDEDIR}/*.xml)

#
# Filter glob
#
foreach(PROTO_RAW ${PROTO_LIST_RAW})
  string(REGEX REPLACE "(^xf86vidmode.xml|^ge.xml|.xml)\$" "" PROTO ${PROTO_RAW})
  if(PROTO AND "${PROTO}" IN_LIST XCB_PROTOS)
    message(STATUS "xpp: including xcb proto ${PROTO_RAW}")
    set(PROTO_LIST ${PROTO_LIST} ${PROTO})
  endif()
endforeach(PROTO_RAW)

#
# Add commands
#
set(PROTO_GEN_DIR "${GEN_SRC}/proto")
file(MAKE_DIRECTORY ${PROTO_GEN_DIR})
set(PROTO_GEN_FILES "")
foreach(PROTO ${PROTO_LIST})
    string(REGEX REPLACE "proto\$" "" PROTO_OUTPUT ${PROTO})
    set(OUTPUT_FILE ${PROTO_GEN_DIR}/${PROTO_OUTPUT}.cppm)
    execute_process(
            OUTPUT_FILE ${OUTPUT_FILE}
            COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/generators/cpp_client.py -p "${PYTHON_XCBGEN}"
            ${XCBPROTO_XCBINCLUDEDIR}/${PROTO}.xml > ${OUTPUT_FILE}
            RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "xcb-gen failed")
    endif()
    #add_custom_command(
    #        OUTPUT ${OUTPUT_FILE}
    #        CODEGEN
    #        COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/generators/cpp_client.py -p "${PYTHON_XCBGEN}"
    #        ${XCBPROTO_XCBINCLUDEDIR}/${PROTO}.xml > ${OUTPUT_FILE})
    set_source_files_properties(${OUTPUT_FILE} PROPERTIES GENERATED TRUE)
    list(APPEND PROTO_GEN_FILES ${OUTPUT_FILE})
endforeach(PROTO)


#
# Create project lib and commands
#
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

add_library(xpp)
add_library(xpp::xpp ALIAS xpp)

set_target_properties(xpp PROPERTIES
        SOVERSION ${PROJECT_VERSION_MAJOR}
        VERSION ${PROJECT_VERSION}
)

set_target_properties(xpp PROPERTIES LINKER_LANGUAGE CXX)

set(DEPENDENCIES "find_dependency(X11 REQUIRED COMPONENTS")
foreach (pkg IN LISTS X11_DEPENDENCIES)
    string(APPEND DEPENDENCIES " ${pkg}")
endforeach ()
string(APPEND DEPENDENCIES ")\n")

generate_export_header(xpp)
target_sources(xpp
        PUBLIC
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
target_link_libraries(xpp PUBLIC ${libraries})
target_compile_features(xpp PUBLIC cxx_std_23)

set(targets_export_name xpp-targets)
write_basic_package_version_file(
  ${PROJECT_BINARY_DIR}/xpp-config-version.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/xpp-config.cmake.in
    ${PROJECT_BINARY_DIR}/xpp-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/xpp)

# Use a namespace because CMake provides better diagnostics for namespaced
# imported targets.
export(TARGETS xpp NAMESPACE xpp::
        FILE ${PROJECT_BINARY_DIR}/${targets_export_name}.cmake)

install(EXPORT ${targets_export_name}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/xpp
        NAMESPACE xpp::
        CXX_MODULES_DIRECTORY .)

install(TARGETS xpp
    EXPORT ${targets_export_name}
    FILE_SET cxx_modules DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/xpp/src
    LIBRARY           DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE           DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES ${PROJECT_BINARY_DIR}/xpp-config-version.cmake ${PROJECT_BINARY_DIR}/xpp-config.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/xpp)

install(FILES ${CMAKE_CURRENT_LIST_DIR}/LICENSE DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/xpp)

add_subdirectory(tests)
add_subdirectory(examples EXCLUDE_FROM_ALL)
