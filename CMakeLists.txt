cmake_minimum_required (VERSION 3.29)

project (deb_lights)

find_package(vsg REQUIRED)
find_package(vsgXchange REQUIRED)

set (CMAKE_CXX_STANDARD 17)
include_directories($ENV{HOME}/projects/tinynurbs/include)

add_executable(pills src/pills.cpp
    src/LightupScene.cpp
    src/MakeAxes.cpp
    src/GetPipelineConfigForLines.cpp
    src/MyBuilder.cpp
    src/MyObject.cpp
    src/DynamicLighting.cpp)
target_link_libraries(pills vsg::vsg vsgXchange::vsgXchange)

add_executable(frac src/frac.cpp)
target_link_libraries(frac vsg::vsg vsgXchange::vsgXchange)
