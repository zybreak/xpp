add_executable(demo01 demo_01.cpp)
target_link_libraries(demo01 PUBLIC xpp::xpp)
target_compile_options(demo01 PUBLIC "-Wno-deprecated-declarations")

add_executable(demo02 demo_02.cpp)
target_link_libraries(demo02 PUBLIC xpp::xpp)
target_compile_options(demo02 PUBLIC "-Wno-deprecated-declarations")
