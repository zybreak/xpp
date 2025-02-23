#
# Create test executable
#
find_package(GTest CONFIG REQUIRED)

include(GoogleTest)

add_executable(xlib-test xlib-test.cpp)
target_link_libraries(xlib-test PRIVATE xpp)

add_executable(event event.cpp)
target_link_libraries(event PRIVATE xpp)

add_executable(requests requests.cpp)
target_link_libraries(requests PRIVATE xpp)

add_executable(iterator iterator.cpp)
target_link_libraries(iterator PRIVATE xpp)

# Unit tests
add_executable(UnitTest)
target_sources(UnitTest PRIVATE unittest.cpp)
target_link_libraries(UnitTest PRIVATE xpp GTest::gtest_main GTest::gmock)
gtest_discover_tests(UnitTest)
