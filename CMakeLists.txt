cmake_minimum_required (VERSION 3.29)

project (deb_lights)

find_package(vsg REQUIRED)
find_package(vsgXchange REQUIRED)

set (CMAKE_CXX_STANDARD 17)
include_directories($ENV{HOME}/projects/tinynurbs/include)

add_executable(pills src/pills.cpp src/LightupScene.cpp src/MakeAxes.cpp)
target_link_libraries(pills vsg::vsg vsgXchange::vsgXchange)
