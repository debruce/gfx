cmake_minimum_required (VERSION 3.29)

project (deb_lights)

find_package(vsg REQUIRED)
find_package(vsgXchange REQUIRED)

set (CMAKE_CXX_STANDARD 17)

add_executable(pills src/pills.cpp)
target_link_libraries(pills vsg::vsg vsgXchange::vsgXchange)
