#
# Create test executable
#
find_package(GTest CONFIG REQUIRED)

include(GoogleTest)

#add_executable(xlib-test xlib-test.cpp)
#target_link_libraries(xlib-test xpp ${DEPS_LIBRARIES})
#target_compile_options(xlib-test PUBLIC ${DEPS_CFLAGS})
#target_link_options(xlib-test PUBLIC ${DEPS_LDFLAGS})
#target_link_directories(xlib-test PUBLIC ${DEPS_LIBRARY_DIRS})
#target_include_directories(xlib-test PUBLIC ${DEPS_INCLUDE_DIRS})

#add_executable(event event.cpp)
#target_link_libraries(event xpp ${DEPS_LIBRARIES})
#target_compile_options(event PUBLIC ${DEPS_CFLAGS})
#target_link_options(event PUBLIC ${DEPS_LDFLAGS})
#target_link_directories(event PUBLIC ${DEPS_LIBRARY_DIRS})
#target_include_directories(event PUBLIC ${DEPS_INCLUDE_DIRS})

#add_executable(requests requests.cpp)
#target_link_libraries(requests xpp ${DEPS_LIBRARIES})
#target_compile_options(requests PUBLIC ${DEPS_CFLAGS})
#target_link_options(requests PUBLIC ${DEPS_LDFLAGS})
#target_link_directories(requests PUBLIC ${DEPS_LIBRARY_DIRS})
#target_include_directories(requests PUBLIC ${DEPS_INCLUDE_DIRS})

#add_executable(iterator iterator.cpp)
#target_link_libraries(iterator xpp ${DEPS_LIBRARIES})
#target_compile_options(iterator PUBLIC ${DEPS_CFLAGS})
#target_link_options(iterator PUBLIC ${DEPS_LDFLAGS})
#target_link_directories(iterator PUBLIC ${DEPS_LIBRARY_DIRS})
#target_include_directories(iterator PUBLIC ${DEPS_INCLUDE_DIRS})

# Unit tests
add_executable(UnitTest)
target_sources(UnitTest PUBLIC unittest.cpp)
target_link_libraries(UnitTest PUBLIC xpp GTest::gtest_main GTest::gmock)
gtest_discover_tests(UnitTest)
